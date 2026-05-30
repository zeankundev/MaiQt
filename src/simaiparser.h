#ifndef SIMAIPARSER_H
#define SIMAIPARSER_H

#include <QString>
#include <QVector>
#include <QMap>

enum class NoteType {
    Tap,        // Normal tap
    Hold,       // Long note
    Slide,      // Slide note
    Touch,      // Touch note
    TouchHold,  // Touch hold note
    Break       // Break note (special tap)
};

struct Note {
    double time;        // In beats
    int lane;           // 0-7 for tap/hold/break, 0-7 for touch lanes
    NoteType type;
    double duration;    // For holds, slides, touch holds
    QString slideType;  // Direction for slides: "l", "r", etc.
    
    Note() : time(0), lane(0), type(NoteType::Tap), duration(0) {}
};

struct ChartMetadata {
    QString title;
    QString artist;
    QString designer;
    double bpm;
    int offset;  // In milliseconds
    
    ChartMetadata() : bpm(120), offset(0) {}
};

class SimaiParser {
public:
    SimaiParser();
    
    // Load Simai file
    bool loadFromFile(const QString& filePath);
    bool loadFromString(const QString& content);
    
    // Save Simai file
    bool saveToFile(const QString& filePath) const;
    QString saveToString() const;
    
    // Chart management
    const ChartMetadata& getMetadata() const { return metadata; }
    void setMetadata(const ChartMetadata& meta) { metadata = meta; }
    
    const QVector<Note>& getNotes() const { return notes; }
    QVector<Note>& getNotes() { return notes; }
    
    // Note operations
    void addNote(const Note& note);
    void removeNote(int index);
    void updateNote(int index, const Note& note);
    void sortNotes();
    
    // Utility
    int getMaxLane() const;
    double getTotalDuration() const;
    
private:
    ChartMetadata metadata;
    QVector<Note> notes;
    
    // Parsing helpers
    NoteType parseNoteType(const QString& typeStr);
    QString noteTypeToString(NoteType type) const;
    static QString escapeString(const QString& str);
    static QString unescapeString(const QString& str);
};

#endif // SIMAIPARSER_H
