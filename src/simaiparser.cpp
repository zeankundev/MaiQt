#include "simaiparser.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>

SimaiParser::SimaiParser() {
    metadata.bpm = 120;
    metadata.offset = 0;
}

bool SimaiParser::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    return loadFromString(content);
}

bool SimaiParser::loadFromString(const QString& content) {
    notes.clear();
    
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        
        // Skip comments and empty lines
        if (trimmed.isEmpty() || trimmed.startsWith("//")) {
            continue;
        }
        
        // Parse metadata lines (starting with &)
        if (trimmed.startsWith("&title=")) {
            metadata.title = unescapeString(trimmed.mid(7));
            continue;
        }
        if (trimmed.startsWith("&artist=")) {
            metadata.artist = unescapeString(trimmed.mid(8));
            continue;
        }
        if (trimmed.startsWith("&designer=")) {
            metadata.designer = unescapeString(trimmed.mid(10));
            continue;
        }
        if (trimmed.startsWith("&bpm=")) {
            metadata.bpm = trimmed.mid(5).toDouble();
            continue;
        }
        if (trimmed.startsWith("&offset=")) {
            metadata.offset = trimmed.mid(8).toInt();
            continue;
        }
        
        // Parse note lines
        QMap<QString, QString> noteData;
        QStringList parts = trimmed.split(';', Qt::SkipEmptyParts);
        
        for (const QString& part : parts) {
            int eqPos = part.indexOf('=');
            if (eqPos > 0) {
                QString key = part.left(eqPos).trimmed();
                QString value = part.mid(eqPos + 1).trimmed();
                noteData[key] = value;
            }
        }
        
        if (noteData.contains("t") && noteData.contains("n") && noteData.contains("c")) {
            Note note;
            note.time = noteData["t"].toDouble();
            note.lane = noteData["n"].toInt();
            note.type = parseNoteType(noteData["c"]);
            
            if (noteData.contains("l")) {
                note.duration = noteData["l"].toDouble();
            }
            
            if (noteData.contains("s")) {
                note.slideType = noteData["s"];
            }
            
            notes.append(note);
        }
    }
    
    sortNotes();
    return true;
}

bool SimaiParser::saveToFile(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << saveToString();
    file.close();
    
    return true;
}

QString SimaiParser::saveToString() const {
    QString output;
    
    // Write metadata
    if (!metadata.title.isEmpty()) {
        output += "&title=" + escapeString(metadata.title) + "\n";
    }
    if (!metadata.artist.isEmpty()) {
        output += "&artist=" + escapeString(metadata.artist) + "\n";
    }
    if (!metadata.designer.isEmpty()) {
        output += "&designer=" + escapeString(metadata.designer) + "\n";
    }
    output += "&bpm=" + QString::number(metadata.bpm) + "\n";
    output += "&offset=" + QString::number(metadata.offset) + "\n";
    output += "\n";
    
    // Write notes
    for (const Note& note : notes) {
        output += QString("t=%1;n=%2;c=%3")
            .arg(note.time, 0, 'f', 3)
            .arg(note.lane)
            .arg(noteTypeToString(note.type));
        
        if (note.duration > 0) {
            output += QString(";l=%1").arg(note.duration, 0, 'f', 3);
        }
        
        if (!note.slideType.isEmpty()) {
            output += QString(";s=%1").arg(note.slideType);
        }
        
        output += "\n";
    }
    
    return output;
}

void SimaiParser::addNote(const Note& note) {
    notes.append(note);
    sortNotes();
}

void SimaiParser::removeNote(int index) {
    if (index >= 0 && index < notes.size()) {
        notes.removeAt(index);
    }
}

void SimaiParser::updateNote(int index, const Note& note) {
    if (index >= 0 && index < notes.size()) {
        notes[index] = note;
        sortNotes();
    }
}

void SimaiParser::sortNotes() {
    std::sort(notes.begin(), notes.end(), [](const Note& a, const Note& b) {
        return a.time < b.time;
    });
}

int SimaiParser::getMaxLane() const {
    return 7;  // Standard maimai has 8 lanes (0-7)
}

double SimaiParser::getTotalDuration() const {
    if (notes.isEmpty()) {
        return 0;
    }
    
    double maxTime = 0;
    for (const Note& note : notes) {
        double endTime = note.time + note.duration;
        maxTime = std::max(maxTime, endTime);
    }
    return maxTime;
}

NoteType SimaiParser::parseNoteType(const QString& typeStr) {
    if (typeStr.startsWith("1")) {
        return NoteType::Tap;
    } else if (typeStr.startsWith("2")) {
        return NoteType::Hold;
    } else if (typeStr.startsWith("3")) {
        return NoteType::Slide;
    } else if (typeStr.startsWith("4")) {
        return NoteType::Touch;
    } else if (typeStr.startsWith("5")) {
        return NoteType::TouchHold;
    } else if (typeStr.startsWith("6")) {
        return NoteType::Break;
    }
    return NoteType::Tap;
}

QString SimaiParser::noteTypeToString(NoteType type) const {
    switch (type) {
        case NoteType::Tap:
            return "1";
        case NoteType::Hold:
            return "2";
        case NoteType::Slide:
            return "3";
        case NoteType::Touch:
            return "4";
        case NoteType::TouchHold:
            return "5";
        case NoteType::Break:
            return "6";
    }
    return "1";
}

QString SimaiParser::escapeString(const QString& str) {
    return str.replace('"', "\\\"").replace('\n', "\\n");
}

QString SimaiParser::unescapeString(const QString& str) {
    return str.replace("\\\"", "\"").replace("\\n", "\n");
}
