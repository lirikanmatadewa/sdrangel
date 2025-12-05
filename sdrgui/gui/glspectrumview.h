///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2015-2022 Edouard Griffiths, F4EXB <f4exb06@gmail.com>          //
// Copyright (C) 2018 beta-tester <alpha-beta-release@gmx.net>                   //
// Copyright (C) 2022-2023 Jon Beniston, M7RCE <jon@beniston.com>                //
// Copyright (C) 2022 Jiří Pinkava <jiri.pinkava@rossum.ai>                      //
//                                                                               //
// OpenGL interface modernization.                                               //
// See: http://doc.qt.io/qt-5/qopenglshaderprogram.html                          //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_GLSPECTRUMVIEW_H
#define INCLUDE_GLSPECTRUMVIEW_H

#include <QTimer>
#include <QMutex>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QPoint>
#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>
#include "gui/qtcompatibility.h"
#include "gui/scaleengine.h"
#include "gui/glshadersimple.h"
#include "gui/glshadertextured.h"
#include "gui/glshadercolormap.h"
#include "dsp/glspectruminterface.h"
#include "gui/glshaderspectrogram.h"
#include "dsp/spectrummarkers.h"
#include "dsp/channelmarker.h"
#include "dsp/spectrumsettings.h"
#include "export.h"
#include "util/incrementalarray.h"
#include "util/message.h"
#include "util/colormap.h"
#include "util/peakfinder.h"

#include <QHash>

class QOpenGLShaderProgram;
class MessageQueue;
class SpectrumVis;
class QOpenGLDebugLogger;
class SpectrumMeasurements;

class SDRGUI_API GLSpectrumView : public QOpenGLWidget, public GLSpectrumInterface {
    Q_OBJECT

public:
    // marker
    void postMarkersChangedToGUI();

    int getHistogramMarkerFollowPeakOrder(int idx) const;

    // marker
    // idx  : index marker (0..N-1)
    // order: 1 = peak tertinggi, 2 = peak kedua, dst
    void setHistogramMarkerFollowPeak(int idx, int order);
    void clearHistogramMarkerFollowPeak(int idx);

    // marker
    float getHistogramLivePowerAtIndex(int idx) const;
    void setHistogramDeltaMode(bool on) { m_histogramDeltaMode = on; }
    bool getHistogramDeltaMode() const { return m_histogramDeltaMode; }
    MessageQueue* getMessageQueueToGUI() { return m_messageQueueToGUI; }

    void setHistogramReferenceIndex(int idx);
    int histogramReferenceIndex() const { return m_histogramReferenceIndex; }

    void setWaterfallReferenceIndex(int idx);
    int waterfallReferenceIndex() const { return m_waterfallReferenceIndex; }

    // Manual span control
    void setManualSpan(qint64 centerHz, int spanLeftHz, int spanRightHz); 
    void clearManualSpan();                                               
    bool isManualSpanEnabled() const { return m_manualSpanEnabled; }

    void enableMultiSlices(const QVector<qint64>& centersHz);
    void clearMultiSlices();
    bool multiSlicesEnabled() const { return m_multiSlicesEnabled; }

    class MsgReportSampleRate : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportSampleRate(quint32 sampleRate) :
            Message(),
            m_sampleRate(sampleRate)
        {}

        quint32 getSampleRate() const { return m_sampleRate; }

    private:
        quint32 m_sampleRate;
    };

    class MsgReportWaterfallShare : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportWaterfallShare(Real waterfallShare) :
            Message(),
            m_waterfallShare(waterfallShare)
        {}

        Real getWaterfallShare() const { return m_waterfallShare; }

    private:
        Real m_waterfallShare;
    };

    class MsgReportFFTOverlap : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportFFTOverlap(int overlap) :
            Message(),
            m_overlap(overlap)
        {}

        int getOverlap() const { return m_overlap; }

    private:
        int m_overlap;
    };

    class MsgReportPowerScale : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportPowerScale(int refLevel, int range) :
            Message(),
            m_refLevel(refLevel),
            m_range(range)
        {}

        Real getRefLevel() const { return m_refLevel; }
        Real getRange() const { return m_range; }

    private:
        Real m_refLevel;
        Real m_range;
    };

    class MsgReportCalibrationShift : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportCalibrationShift(Real calibrationShiftdB) :
            Message(),
            m_calibrationShiftdB(calibrationShiftdB)
        {}

        Real getCalibrationShiftdB() const { return m_calibrationShiftdB; }
    private:
        Real m_calibrationShiftdB;
    };

    class MsgReportHistogramMarkersChange : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportHistogramMarkersChange() :
            Message()
        {}
    };

    class MsgReportWaterfallMarkersChange : public Message {
        MESSAGE_CLASS_DECLARATION

    public:
        MsgReportWaterfallMarkersChange() :
            Message()
        {}
    };

    // marker
    class MsgReportLivePowersTick : public Message {
        MESSAGE_CLASS_DECLARATION
    public:
        MsgReportLivePowersTick() : Message() {}
    };

    GLSpectrumView(QWidget* parent = nullptr);
    virtual ~GLSpectrumView();

    void setCenterFrequency(qint64 frequency);
    qint64 getCenterFrequency() const { return m_centerFrequency; }
    float getPowerMax() const;
    float getTimeMax() const;
    void setSampleRate(qint32 sampleRate);
    void setTimingRate(qint32 timingRate);
    void setFFTOverlap(int overlap);
    void setReferenceLevel(Real referenceLevel);
    void setPowerRange(Real powerRange);
    void setDecay(int decay);
    void setDecayDivisor(int decayDivisor);
    void setHistoStroke(int stroke);
    void setDisplayWaterfall(bool display);
    void setDisplay3DSpectrogram(bool display);
    void set3DSpectrogramStyle(SpectrumSettings::SpectrogramStyle style);
    void setColorMapName(const QString &colorMapName);
    void setSpectrumStyle(SpectrumSettings::SpectrumStyle style);
    void setSsbSpectrum(bool ssbSpectrum);
    void setLsbDisplay(bool lsbDisplay);
    void setInvertedWaterfall(bool inv);
    void setDisplayMaxHold(bool display);
    void setDisplayCurrent(bool display);
    void setDisplayHistogram(bool display);
    void setDisplayGrid(bool display);
    void setDisplayGridIntensity(int intensity);
    void setDisplayTraceIntensity(int intensity);
    void setFreqScaleTruncationMode(bool mode);
    void setLinear(bool linear);
    void setUseCalibration(bool useCalibration);
    void setMeasurements(SpectrumMeasurements *measurements) { m_measurements = measurements; }
    void setMeasurementParams(SpectrumSettings::Measurement measurement,
                              int centerFrequencyOffset, int bandwidth, int chSpacing, int adjChBandwidth,
                              int harmonics, int peaks, bool highlight, int precision);
    qint32 getSampleRate() const { return m_sampleRate; }

    void addChannelMarker(ChannelMarker* channelMarker);
    void removeChannelMarker(ChannelMarker* channelMarker);
    void setMessageQueueToGUI(MessageQueue* messageQueue) { m_messageQueueToGUI = messageQueue; }

    virtual void newSpectrum(const Real* spectrum, int nbBins, int fftSize);
    void clearSpectrumHistogram();

    Real getWaterfallShare() const { return m_waterfallShare; }
    void setWaterfallShare(Real waterfallShare);
    void setFPSPeriodMs(int fpsPeriodMs);

    void setDisplayedStream(bool sourceOrSink, int streamIndex)
    {
        m_displaySourceOrSink = sourceOrSink;
        m_displayStreamIndex = streamIndex;
    }
    void setSpectrumVis(SpectrumVis *spectrumVis) { m_spectrumVis = spectrumVis; }
    SpectrumVis *getSpectrumVis() { return m_spectrumVis; }
    const QList<SpectrumHistogramMarker>& getHistogramMarkers() const { return m_histogramMarkers; }
    QList<SpectrumHistogramMarker>& getHistogramMarkers() { return m_histogramMarkers; }
    void setHistogramMarkers(const QList<SpectrumHistogramMarker>& histogramMarkers);
    const QList<SpectrumWaterfallMarker>& getWaterfallMarkers() const { return m_waterfallMarkers; }
    QList<SpectrumWaterfallMarker>& getWaterfallMarkers() { return m_waterfallMarkers; }
    void setWaterfallMarkers(const QList<SpectrumWaterfallMarker>& waterfallMarkers);
    const QList<SpectrumAnnotationMarker>& getAnnotationMarkers() const { return m_annotationMarkers; }
    QList<SpectrumAnnotationMarker>& getAnnotationMarkers() { return m_annotationMarkers; }
    void setAnnotationMarkers(const QList<SpectrumAnnotationMarker>& annotationMarkers);
    void updateHistogramMarkers();
    void updateHistogramPeaks();
    void updateWaterfallMarkers();
    void updateAnnotationMarkers();
    void updateMarkersDisplay();
    void updateCalibrationPoints();
    SpectrumSettings::MarkersDisplay& getMarkersDisplay() { return m_markersDisplay; }
    bool& getHistogramFindPeaks() { return m_histogramFindPeaks; }
    void setHistogramFindPeaks(bool value) { m_histogramFindPeaks = value; }
	void setMarkersDisplay(SpectrumSettings::MarkersDisplay markersDisplay);
    QList<SpectrumCalibrationPoint>& getCalibrationPoints() { return m_calibrationPoints; }
    void setCalibrationPoints(const QList<SpectrumCalibrationPoint>& calibrationPoints);
    SpectrumSettings::CalibrationInterpolationMode& getCalibrationInterpMode() { return m_calibrationInterpMode; }
    void setCalibrationInterpMode(SpectrumSettings::CalibrationInterpolationMode mode);
    void setIsDeviceSpectrum(bool isDeviceSpectrum) { m_isDeviceSpectrum = isDeviceSpectrum; }
    bool isDeviceSpectrum() const { return m_isDeviceSpectrum; }

private:

    // marker
    enum class DragTarget { None, Histo, Wat, AnnoStart, AnnoCenter };
    QVector<int> m_markerLastPeakOrder;

    QHash<int, int> m_markerFollowPeakOrder;

    int m_histogramReferenceIndex = 0;
    int m_waterfallReferenceIndex = 0;

    DragTarget m_dragTarget = DragTarget::None;
    int   m_dragIndex = -1;
    QPointF m_dragStartPx;
    qint64  m_dragStartFreq = 0;
    float   m_dragStartPower = 0.0f;
    float   m_dragStartTime = 0.0f;
    float   m_markerGrabTolPx = 6.0f;
    int  hitTestHistogramMarker(const QPointF& pLocalPx) const;
    int  hitTestWaterfallMarker(const QPointF& pLocalPx) const;
    std::pair<int, DragTarget> hitTestAnnotationMarker(const QPointF& pLocalPx) const;
    inline float clamp01(float v) const { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
    inline float normXHistogram(float px) const {
        return (px / width() - m_histogramRect.left()) / m_histogramRect.width();
    }
    inline float normXWaterfall(float px) const {
        return (px / width() - m_waterfallRect.left()) / m_waterfallRect.width();
    }
    inline int   xFromFreqHistogram(qint64 f) const {
        float x = (float)((f - m_frequencyScale.getRangeMin()) / m_frequencyScale.getRange());
        return int((m_histogramRect.left() + x * m_histogramRect.width()) * width());
    }
    inline int   xFromFreqWaterfall(qint64 f) const {
        float x = (float)((f - m_frequencyScale.getRangeMin()) / m_frequencyScale.getRange());
        return int((m_waterfallRect.left() + x * m_waterfallRect.width()) * width());
    }

    bool m_histogramDeltaMode = false;

    // Manual span state
    bool   m_manualSpanEnabled = false;
    qint64 m_manualCenterHz = 0;
    int    m_manualLeftHz = 0;
    int    m_manualRightHz = 0;

    // --- header ---
    struct ExtSlice {
        qint64              centerHz = 0;
        qint32              sampleRate = 0;
        int                 fftSize = 0;
        QVector<Real>       data;
        bool                hasData = false;
        quint64             tick = 0;
    };

    // --- Multi-slices state ---
    bool m_multiSlicesEnabled = false;
    QVector<ExtSlice> m_slices;
    QHash<qint64, int> m_idxByCF;
    QVector<Real> m_multiComposite;

    // (opsional) deteksi 1 siklus selesai:
    QSet<qint64> m_visitedCFs;
    qint64 m_firstCFSeen = 0;
    bool   m_lockCapture = false;       // kalau true, stop overwrite setelah 1 siklus

    bool m_multiCompositeDirty = false;

    struct CycleSlice {
        qint32        sampleRate = 0;
        int           fftSize = 0;
        QVector<Real> data;
    };

    QHash<qint64, CycleSlice> m_cycleSlices;   // data per CF aktual dalam siklus berjalan
    QSet<qint64>              m_cycleCFs;      // set CF yang sudah dikumpulkan
    qint64                    m_prevCF = std::numeric_limits<qint64>::min();
    int                       m_cycleDir = 0;  // +1: naik, -1: turun, 0: belum tahu

    struct ChannelMarkerState {
        ChannelMarker* m_channelMarker;
        QMatrix4x4 m_glMatrixWaterfall;
        QMatrix4x4 m_glMatrixDsbWaterfall;
        QMatrix4x4 m_glMatrixFreqScale;
        QMatrix4x4 m_glMatrixDsbFreqScale;
        QMatrix4x4 m_glMatrixHistogram;
        QMatrix4x4 m_glMatrixDsbHistogram;
        QRectF m_rect;

        ChannelMarkerState(ChannelMarker* channelMarker) :
            m_channelMarker(channelMarker)
        { }
    };
    QList<ChannelMarkerState*> m_channelMarkerStates;

    enum CursorState {
        CSNormal,
        CSSplitter,
        CSSplitterMoving,
        CSChannel,
        CSChannelMoving
    };

    QList<SpectrumHistogramMarker> m_histogramMarkers;
    QList<SpectrumWaterfallMarker> m_waterfallMarkers;
    QList<SpectrumAnnotationMarker> m_annotationMarkers;
    QList<SpectrumAnnotationMarker*> m_sortedAnnotationMarkers;
    QList<SpectrumAnnotationMarker*> m_visibleAnnotationMarkers;
    SpectrumSettings::MarkersDisplay m_markersDisplay;
    bool m_histogramFindPeaks;
    PeakFinder m_peakFinder;
    QList<SpectrumCalibrationPoint> m_calibrationPoints;

    CursorState m_cursorState;
    int m_cursorChannel;

    SpectrumVis* m_spectrumVis;
    QTimer m_timer;
    int m_fpsPeriodMs;
    QMutex m_mutex;
    bool m_mouseInside;
    bool m_changesPending;

    qint64 m_centerFrequency;
    Real m_referenceLevel;
    Real m_powerRange;
    bool m_linear;
    int m_decay;
    quint32 m_sampleRate;
    quint32 m_timingRate;
    int m_fftOverlap;

    int m_fftSize; //!< FFT size in number of bins
    int m_nbBins;  //!< Number of visible FFT bins (zoom support)

    bool m_displayGrid;
    int m_displayGridIntensity;
    int m_displayTraceIntensity;
    bool m_invertedWaterfall;

    std::vector<Real> m_maxHold;
    bool m_displayMaxHold;
    const Real *m_currentSpectrum;
    bool m_displayCurrent;

    Real m_waterfallShare;

    int m_leftMargin;
    int m_rightMargin;
    int m_topMargin;
    int m_frequencyScaleHeight;
    int m_infoHeight;
    int m_histogramHeight;
    int m_waterfallHeight;
    int m_bottomMargin;
    QFont m_textOverlayFont;
    QPixmap m_leftMarginPixmap;
    QPixmap m_frequencyPixmap;
    QPixmap m_infoPixmap;
    ScaleEngine m_timeScale;
    ScaleEngine m_powerScale;
    ScaleEngine m_frequencyScale;
    QRectF m_histogramRect;
    QRect m_frequencyScaleRect;
    QRectF m_waterfallRect;
    QRect m_infoRect;
    QMatrix4x4 m_glFrequencyScaleBoxMatrix;
    QMatrix4x4 m_glLeftScaleBoxMatrix;
    QMatrix4x4 m_glInfoBoxMatrix;

    QString m_peakFrequencyMaxStr;
    QString m_peakPowerMaxStr;
    QString m_peakPowerUnits;

    QRgb m_waterfallPalette[240];
    QImage* m_waterfallBuffer;
    int m_waterfallBufferPos;
    int m_waterfallTextureHeight;
    int m_waterfallTexturePos;
    QMatrix4x4 m_glWaterfallBoxMatrix;
    bool m_displayWaterfall;
    bool m_ssbSpectrum;
    bool m_lsbDisplay;

    QImage* m_3DSpectrogramBuffer;
    int m_3DSpectrogramBufferPos;
    int m_3DSpectrogramTextureHeight;
    int m_3DSpectrogramTexturePos;
    bool m_display3DSpectrogram;
    bool m_rotate3DSpectrogram;     //!< Set when mouse is pressed in 3D spectrogram area for rotation when mouse is moved
    bool m_pan3DSpectrogram;
    bool m_scaleZ3DSpectrogram;
    QPointF m_mousePrevLocalPos;    //!< Position of the mouse for last event
    int m_3DSpectrogramBottom;
    QPixmap m_spectrogramTimePixmap;
    QPixmap m_spectrogramPowerPixmap;
    SpectrumSettings::SpectrogramStyle m_3DSpectrogramStyle;
    QString m_colorMapName;
    SpectrumSettings::SpectrumStyle m_spectrumStyle;
    const float *m_colorMap;

    bool m_scrollFrequency;
    qint64 m_scrollStartCenterFreq;
    bool m_pinching;
    bool m_pinching3D;
    QPointF m_pinchStart;

    bool m_frequencyRequested;      //!< Set when we have emitted requestCenterFrequency
    qint64 m_requestedFrequency;
    qint64 m_nextFrequency;         //!< Next frequency to request when previous request completes
    qint64 m_nextFrequencyValid;

    QRgb m_histogramPalette[240];
    QImage* m_histogramBuffer;
    quint8* m_histogram; //!< Spectrum phosphor matrix of FFT width and PSD height scaled to 100. values [0..239]
    int m_decayDivisor;
    int m_decayDivisorCount;
    int m_histogramStroke;
    QMatrix4x4 m_glHistogramSpectrumMatrix;
    QMatrix4x4 m_glHistogramBoxMatrix;
    bool m_displayHistogram;
    bool m_displayChanged;
    bool m_displaySourceOrSink;
    int m_displayStreamIndex;
    float m_frequencyZoomFactor;
    float m_frequencyZoomPos;
    static const float m_maxFrequencyZoom;
    static const float m_annotationMarkerHeight;

    GLShaderSimple m_glShaderSimple;
    GLShaderTextured m_glShaderLeftScale;
    GLShaderTextured m_glShaderFrequencyScale;
    GLShaderTextured m_glShaderWaterfall;
    GLShaderTextured m_glShaderHistogram;
    GLShaderColorMap m_glShaderColorMap;
    GLShaderTextured m_glShaderTextOverlay;
    GLShaderTextured m_glShaderInfo;
    GLShaderSpectrogram m_glShaderSpectrogram;
    GLShaderTextured m_glShaderSpectrogramTimeScale;
    GLShaderTextured m_glShaderSpectrogramPowerScale;
    int m_matrixLoc;
    int m_colorLoc;
    bool m_useCalibration;
    Real m_calibrationGain;
    Real m_calibrationShiftdB;
    SpectrumSettings::CalibrationInterpolationMode m_calibrationInterpMode;
    IncrementalArray<GLfloat> m_q3TickTime;
    IncrementalArray<GLfloat> m_q3TickFrequency;
    IncrementalArray<GLfloat> m_q3TickPower;
    IncrementalArray<GLfloat> m_q3FFT;
    IncrementalArray<GLfloat> m_q3ColorMap;

    MessageQueue *m_messageQueueToGUI;
    QOpenGLDebugLogger *m_openGLLogger;
    bool m_isDeviceSpectrum;

    SpectrumMeasurements *m_measurements;
    SpectrumSettings::Measurement m_measurement;
    int m_measurementCenterFrequencyOffset;
    int m_measurementBandwidth;
    int m_measurementChSpacing;
    int m_measurementAdjChBandwidth;
    int m_measurementHarmonics;
    int m_measurementPeaks;
    bool m_measurementHighlight;
    int m_measurementPrecision;
    static const QVector4D m_measurementLightMarkerColor;
    static const QVector4D m_measurementDarkMarkerColor;

#ifdef ENABLE_PROFILER
    QString m_profileName;
#endif

    void updateWaterfall(const Real *spectrum);
    void update3DSpectrogram(const Real *spectrum);
    void updateHistogram(const Real *spectrum);

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void drawPowerBandMarkers(float max, float min, const QVector4D &color);
    void drawBandwidthMarkers(int64_t centerFrequency, int bandwidth, const QVector4D &color);
    void drawPeakMarkers(int64_t startFrequency, int64_t endFrequency, const QVector4D &color);
    void drawSpectrumMarkers();
    void drawAnnotationMarkers();

    void measurePeak();
    void measurePeaks();
    void measureChannelPower();
    void measureAdjacentChannelPower();
    void measureOccupiedBandwidth();
    void measure3dBBandwidth();
    void measureSNR();
    void measureSFDR();
    float calcChannelPower(int64_t centerFrequency, int channelBandwidth) const;
    float calPower(float power) const;
    int findPeakBin(const Real *spectrum) const;
    void findPeak(float &power, float &frequency) const;
    void peakWidth(const Real *spectrum, int center, int &left, int &right, int maxLeft, int maxRight) const;
    int frequencyToBin(int64_t frequency) const;
    int64_t binToFrequency(int bin) const;

    void stopDrag();
    void applyChanges();

    bool event(QEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent*);
    void channelMarkerMove(QWheelEvent*, int mul);
    void zoomFactor(const QPointF& p, float factor);
    void zoom(const QPointF& p, int y);
    void frequencyZoom(float pw);
    void frequencyPan(QMouseEvent*);
    void timeZoom(bool zoomInElseOut);
    void powerZoom(float pw, bool zoomInElseOut);
    void resetFrequencyZoom();
    void updateFFTLimits();
    void setFrequencyScale();
    void setPowerScale(int height);
    void getFrequencyZoom(int64_t& centerFrequency, int& frequencySpan);
    bool pointInWaterfallOrSpectrogram(const QPointF &point) const;
    bool pointInHistogram(const QPointF &point) const;

    void enterEvent(EnterEventType* event);
    void leaveEvent(QEvent* event);

    static QString displayFull(int64_t value);
    static QString displayScaled(int64_t value, char type, int precision, bool showMult);
    static QString displayScaledF(float value, char type, int precision, bool showMult);
    static QString displayPower(float value, char type, int precision);
    int getPrecision(int value);
    void drawTextRight(const QString &text, const QString &value, const QString &max, const QString &units);
    void drawTextsRight(const QStringList &text, const QStringList &value, const QStringList &max, const QStringList &units);
    void drawTextOverlayCentered(
            const QString& text,
            const QColor& color,
            const QFont& font,
            float shiftX,
            float shiftY,
            const QRectF& glRect);
    void drawTextOverlay(      //!< Draws a text overlay
            const QString& text,
            const QColor& color,
            const QFont& font,
            float shiftX,
            float shiftY,
            bool leftHalf,
            bool topHalf,
            const QRectF& glRect);
    void formatTextInfo(QString& info);
    void updateSortedAnnotationMarkers();
    void queueRequestCenterFrequency(qint64 frequency);

    static bool annotationDisplayLessThan(const SpectrumAnnotationMarker *m1, const SpectrumAnnotationMarker *m2)
    {
        if (m1->m_bandwidth == m2->m_bandwidth) {
            return m1->m_startFrequency < m2->m_startFrequency;
        } else {
            return m1->m_bandwidth > m2->m_bandwidth; // larger bandwidths should come first for display (lower layer)
        }
    }

    static bool calibrationPointsLessThan(const SpectrumCalibrationPoint& m1, const SpectrumCalibrationPoint& m2)
    {
        return m1.m_frequency < m2.m_frequency;
    }

private slots:
    void cleanup();
    void tick();
    void channelMarkerChanged();
    void channelMarkerDestroyed(QObject* object);
    void openGLDebug(const QOpenGLDebugMessage &debugMessage);
    bool eventFilter(QObject *object, QEvent *event);

signals:
    // Emitted when user tries to scroll to frequency currently out of range
    void requestCenterFrequency(qint64 frequency);
    // Emitted when annotations are changed
    void updateAnnotations();
    // Emitted when user ctrl-clicks on waterfall to select a time. time is in seconds.
    void timeSelected(float time);

    void multiCaptureCycleDone();


};

#endif // INCLUDE_GLSPECTRUMVIEW_H
