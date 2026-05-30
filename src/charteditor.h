#ifndef CHARTEDITOR_H
#define CHARTEDITOR_H

#include <QWidget>
#include <QScrollArea>
#include <QPainter>
#include "simaiparser.h"

class ChartEditorCanvas : public QWidget {
    Q_OBJECT

public:
    ChartEditorCanvas(SimaiParser* parser, QWidget* parent = nullptr);
    
    void setCurrentTime(double time);
    double getCurrentTime() const { return currentTime; }
    
    void setZoom(double zoom);
    double getZoom() const { return zoomLevel; }
    
    void setBPM(double bpm);
    
    // Note editing
    void addNoteAtTime(double time, int lane, NoteType type);
    void removeSelectedNote();
    void setSelectedNoteType(NoteType type);
    
    int getSelectedNoteIndex() const { return selectedNoteIndex; }
    void setSelectedNoteIndex(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    SimaiParser* parser;
    double currentTime;
    double zoomLevel;  // Pixels per beat
    double bpm;
    int selectedNoteIndex;
    bool draggingNote;
    
    // Constants for rendering
    static constexpr int LANE_WIDTH = 60;
    static constexpr int LANE_HEIGHT = 50;
    static constexpr int HEADER_HEIGHT = 30;
    static constexpr int TIMELINE_WIDTH = 40;
    
    // Helper functions
    QRect getNoteRect(const Note& note) const;
    int getLaneAtPosition(int x) const;
    double getTimeAtPosition(int y) const;
    int getYPositionForTime(double time) const;
    
    void drawGrid(QPainter& painter);
    void drawTimeline(QPainter& painter);
    void drawLanes(QPainter& painter);
    void drawNotes(QPainter& painter);
    void drawPlayhead(QPainter& painter);
    
    QColor getNoteColor(NoteType type) const;
};

class ChartEditor : public QScrollArea {
    Q_OBJECT

public:
    ChartEditor(SimaiParser* parser, QWidget* parent = nullptr);
    
private:
    ChartEditorCanvas* canvas;
};

#endif // CHARTEDITOR_H
