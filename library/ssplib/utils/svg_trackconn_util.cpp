#include "svg_trackconn_util.h"

#include <QVector>

using namespace ssplib;

const char trackSideLetters[int(ssplib::Side::NSides)] = {
    'W', //ssplib::Side::West
    'E' //ssplib::Side::East
};

static int parseInteger(const QString& str, int &pos)
{
    int val = 0;
    for(; pos < str.size(); pos++)
    {
        if(str.at(pos).isDigit())
        {
            val *= 10;
            val += str.at(pos).digitValue();
        }

        if(str.at(pos) == ',' || str.at(pos) == ')')
            break;
    }
    return val;
}

bool utils::parseTrackConnectionAttribute(const QString &value, QVector<TrackConnectionInfo> &outVec)
{
    TrackConnectionInfo info;

    enum class Section
    {
        OutsideValue,
        GateLetter,
        GateTrack,
        StationTrack,
        StationTrackSide
    };

    Section section = Section::OutsideValue;

    for(int i = 0; i < value.size(); i++)
    {
        //Skip spaces and commas
        while (i < value.size() && (value.at(i).isSpace() || value.at(i) == ','))
        {
            i++;
        }

        if(i >= value.size())
            break;

        switch (section)
        {
        case Section::OutsideValue:
        {
            if(value.at(i) == '(')
            {
                section = Section::GateLetter;
            }

            break;
        }
        case Section::GateLetter:
        {
            if(value.at(i).isLetter())
            {
                info.gateLetter = value.at(i).toUpper();
                section = Section::GateTrack;
            }

            break;
        }
        case Section::GateTrack:
        {
            int num = parseInteger(value, i);
            info.gateTrackPos = num;
            section = Section::StationTrack;

            break;
        }
        case Section::StationTrack:
        {
            int num = parseInteger(value, i);
            info.stationTrackPos = num;
            section = Section::StationTrackSide;

            //Skip spaces and commas
            while (i < value.size() && (value.at(i).isSpace() || value.at(i) == ','))
            {
                i++;
            }

            if(i < value.size() && value.at(i) == ')')
            {
                //NOTE: we ignore StationTrackSide if not present to keep compatibility
                //with old format
                //Store value and restart parsing
                outVec.append(info);
                section = Section::OutsideValue;
            }else{
                //NOTE: go back by 1 so next loop goes forward by 1
                //And gets to StationTrackSide to parse it.
                i--;
            }

            break;
        }
        case Section::StationTrackSide:
        {
            const char sideLetter = value.at(i).toUpper().toLatin1();
            if(sideLetter == trackSideLetters[int(ssplib::Side::East)])
            {
                info.trackSide = ssplib::Side::East;
            }
            else if(sideLetter == trackSideLetters[int(ssplib::Side::West)])
            {
                info.trackSide = ssplib::Side::West;
            }
            else
            {
                info.trackSide = ssplib::Side::NSides;
            }
            i++;

            //Skip spaces and commas
            while (i < value.size() && (value.at(i).isSpace() || value.at(i) == ','))
            {
                i++;
            }

            if(i < value.size() && value.at(i) == ')')
            {
                //Store value and restart parsing
                outVec.append(info);
                section = Section::OutsideValue;
            }

            break;
        }
        }
    }

    return true;
}

QString utils::trackConnInfoToString(const QVector<TrackConnectionInfo> &vec)
{
    QString value;
    value.reserve(8 * vec.size());

    for(const TrackConnectionInfo& info : vec)
    {
        value += '(';
        value += info.gateLetter;
        value += ',';
        value += QString::number(info.gateTrackPos);
        value += ',';
        value += QString::number(info.stationTrackPos);
        value += ',';
        const int trackSide = int(info.trackSide);
        if(trackSide >= int(ssplib::Side::NSides))
            value += '?';
        else
            value += trackSideLetters[trackSide];
        value += "),";
    }
    if(!value.isEmpty() && value.endsWith(','))
        value.chop(1); //Remove last comma

    return value;
}
