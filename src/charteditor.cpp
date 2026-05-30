#include "charteditor.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <cmath>

ChartEditorCanvas::ChartEditorCanvas(SimaiParser* parser, QWidget* parent)
    : QWidget(parent), parser(parser), currentTime(0), zoomLevel(30),
      bpm(120), selectedNoteIndex(-1), draggingNote(false) {
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
}

void ChartEditorCanvas::setCurrentTime(double time) {
    currentTime = time;
    update();
}

void ChartEditorCanvas::setZoom(double zoom) {
    zoomLevel = qMax(5.0, qMin(100.0, zoom));
    setMinimumWidth(int(parser->getTotalDuration() * zoomLevel) + TIMELINE_WIDTH + 100);
    update();
}

void ChartEditorCanvas::setBPM(double newBpm) {
    bpm = newBpm;
}

void ChartEditorCanvas::addNoteAtTime(double time, int lane, NoteType type) {
    if (lane >= 0 && lane <= parser->getMaxLane()) {
        Note note;
        note.time = time;
        note.lane = lane;
        note.type = type;
        note.duration = 0;
        parser->addNote(note);
        update();
    }
}

void ChartEditorCanvas::removeSelectedNote() {
    if (selectedNoteIndex >= 0 && selectedNoteIndex < parser->getNotes().size()) {
        parser->removeNote(selectedNoteIndex);
        selectedNoteIndex = -1;
        update();
    }
}

void ChartEditorCanvas::setSelectedNoteType(NoteType type) {
    if (selectedNoteIndex >= 0 && selectedNoteIndex < parser->getNotes().size()) {
        parser->getNotes()[selectedNoteIndex].type = type;
        update();
    }
}

void ChartEditorCanvas::setSelectedNoteIndex(int index) {
    selectedNoteIndex = index;
    update();
}

int ChartEditorCanvas::getLaneAtPosition(int x) const {
    if (x < TIMELINE_WIDTH) return -1;
    int laneX = x - TIMELINE_WIDTH;
    return laneX / LANE_WIDTH;
}

double ChartEditorCanvas::getTimeAtPosition(int y) const {
    if (y < HEADER_HEIGHT) return -1;
    return (y - HEADER_HEIGHT) / zoomLevel;
}

int ChartEditorCanvas::getYPositionForTime(double time) const {
    return int(time * zoomLevel) + HEADER_HEIGHT;
}

QRect ChartEditorCanvas::getNoteRect(const Note& note) const {
    int x = TIMELINE_WIDTH + note.lane * LANE_WIDTH + 2;
    int y = getYPositionForTime(note.time);
    int w = LANE_WIDTH - 4;
    int h = int(note.duration * zoomLevel);
    
    if (h == 0) h = 20;  // Minimum height for tap notes
    
    return QRect(x, y, w, h);
}

QColor ChartEditorCanvas::getNoteColor(NoteType type) const {
    switch (type) {
        case NoteType::Tap:
            return QColor(100, 200, 255);       // Light blue
        case NoteType::Hold:
            return QColor(255, 200, 100);       // Orange
        case NoteType::Slide:
            return QColor(200, 100, 255);       // Purple
        case NoteType::Touch:
            return QColor(100, 255, 200);       // Cyan
        case NoteType::TouchHold:
            return QColor(255, 255, 100);       // Yellow
        case NoteType::Break:
            return QColor(255, 100, 100);       // Red
    }
    return QColor(128, 128, 128);
}

void ChartEditorCanvas::drawGrid(QPainter& painter) {
    painter.setPen(QPen(QColor(60, 60, 60), 1));
    
    double beatHeight = zoomLevel;
    int startY = HEADER_HEIGHT;
    int endY = height();
    
    for (int i = 0; i * beatHeight <= endY - startY; ++i) {
        int y = startY + int(i * beatHeight);
        if (y < endY) {
            if (i % 4 == 0) {
                painter.setPen(QPen(QColor(100, 100, 100), 2));
            } else {
                painter.setPen(QPen(QColor(60, 60, 60), 1));
            }
            painter.drawLine(TIMELINE_WIDTH, y, width(), y);
        }
    }
}

void ChartEditorCanvas::drawTimeline(QPainter& painter) {
    painter.fillRect(0, 0, TIMELINE_WIDTH, height(), QColor(40, 40, 40));
    painter.setPen(QColor(200, 200, 200));
    painter.setFont(QFont("Arial", 8));
    
    double beatHeight = zoomLevel;
    int startY = HEADER_HEIGHT;
    int endY = height();
    
    for (int i = 0; i * beatHeight <= endY - startY; ++i) {
        int y = startY + int(i * beatHeight);
        if (y < endY) {
            if (i % 4 == 0) {
                painter.drawText(5, y - 2, TIMELINE_WIDTH - 5, 20, Qt::AlignRight | Qt::AlignTop,
                               QString::number(i / 4));
            }
        }
    }
}

void ChartEditorCanvas::drawLanes(QPainter& painter) {
    painter.setPen(QPen(QColor(100, 100, 100), 1));
    
    for (int i = 0; i <= parser->getMaxLane() + 1; ++i) {
        int x = TIMELINE_WIDTH + i * LANE_WIDTH;
        painter.drawLine(x, HEADER_HEIGHT, x, height());
    }
}

void ChartEditorCanvas::drawNotes(QPainter& painter) {
    const auto& notes = parser->getNotes();
    
    for (int i = 0; i < notes.size(); ++i) {
        const Note& note = notes[i];
        QRect rect = getNoteRect(note);
        
        if (rect.top() > height()) break;
        if (rect.bottom() < HEADER_HEIGHT) continue;
        
        // Draw note
        painter.fillRect(rect, getNoteColor(note.type));
        
        // Draw selection highlight
        if (i == selectedNoteIndex) {
            painter.setPen(QPen(QColor(255, 255, 0), 3));
            painter.drawRect(rect);
        } else {
            painter.setPen(QPen(QColor(50, 50, 50), 1));
            painter.drawRect(rect);
        }
        
        // Draw note type indicator
        painter.setPen(QColor(255, 255, 255));
        painter.setFont(QFont("Arial", 7));
        QString typeStr;
        switch (note.type) {
            case NoteType::Tap: typeStr = "T"; break;
            case NoteType::Hold: typeStr = "H"; break;
            case NoteType::Slide: typeStr = "S"; break;
            case NoteType::Touch: typeStr = "To"; break;
            case NoteType::TouchHold: typeStr = "TH"; break;
            case NoteType::Break: typeStr = "B"; break;
        }
        painter.drawText(rect, Qt::AlignCenter, typeStr);
    }
}

void ChartEditorCanvas::drawPlayhead(QPainter& painter) {
    int y = getYPositionForTime(currentTime);
    painter.setPen(QPen(QColor(255, 0, 0), 2));
    painter.drawLine(0, y, width(), y);
}

void ChartEditorCanvas::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    drawGrid(painter);
    drawTimeline(painter);
    drawLanes(painter);
    drawNotes(painter);
    drawPlayhead(painter);
}

void ChartEditorCanvas::mousePressEvent(QMouseEvent* event) {
    int lane = getLaneAtPosition(event->pos().x());
    double time = getTimeAtPosition(event->pos().y());
    
    if (lane < 0 || time < 0) return;
    
    // Check if clicking on existing note
    bool foundNote = false;
    const auto& notes = parser->getNotes();
    for (int i = 0; i < notes.size(); ++i) {
        if (getNoteRect(notes[i]).contains(event->pos())) {
            selectedNoteIndex = i;
            draggingNote = true;
            foundNote = true;
            break;
        }
    }
    
    if (!foundNote) {
        // Add new note
        addNoteAtTime(time, lane, NoteType::Tap);
        selectedNoteIndex = parser->getNotes().size() - 1;
    }
}

void ChartEditorCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (draggingNote && selectedNoteIndex >= 0 && selectedNoteIndex < parser->getNotes().size()) {
        double time = getTimeAtPosition(event->pos().y());
        if (time >= 0) {
            parser->getNotes()[selectedNoteIndex].time = time;
            parser->sortNotes();
            update();
        }
    }
}

void ChartEditorCanvas::mouseReleaseEvent(QMouseEvent* event) {
    draggingNote = false;
}

void ChartEditorCanvas::wheelEvent(QWheelEvent* event) {
    double delta = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    setZoom(zoomLevel * delta);
}

ChartEditor::ChartEditor(SimaiParser* parser, QWidget* parent)
    : QScrollArea(parent) {
    canvas = new ChartEditorCanvas(parser, this);
    setWidget(canvas);
    setStyleSheet("QScrollArea { background-color: #1e1e1e; }");
}
