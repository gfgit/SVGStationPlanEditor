#ifndef SVGTRA_H
#define SVGTRA_H

#include <QString>

namespace ssplib {

typedef qint64 db_id;

enum class Side : qint8
{
    West = 0,
    East = 1,
    NSides
};

struct TrackConnectionInfo
{
    db_id trackId = 0;
    db_id gateId = 0;
    int stationTrackPos = 0;
    int gateTrackPos = 0;
    QChar gateLetter;
    Side trackSide = Side::NSides;

    //NOTE: this matches Database Data
    inline bool matchIDs(const TrackConnectionInfo& other) const
    {
        return trackId == other.trackId && gateId == other.gateId
               && gateTrackPos == other.gateTrackPos && trackSide == other.trackSide;
    }

    //NOTE: this matches SVG Data
    inline bool matchNames(const TrackConnectionInfo& other) const
    {
        return stationTrackPos == other.stationTrackPos && gateLetter == other.gateLetter
               && gateTrackPos == other.gateTrackPos && trackSide == other.trackSide;
    }

    template <typename Container>
    static inline void removeAllNames(Container &vec, const TrackConnectionInfo& info)
    {
        auto it =std::remove_if(vec.begin(), vec.end(),
                                 [info](const TrackConnectionInfo& other) -> bool
                                 {
                                     return info.matchNames(other);
                                 });
        vec.erase(it, vec.end());
    }
};

namespace utils {

bool parseTrackConnectionAttribute(const QString& value, QVector<TrackConnectionInfo>& outVec);

QString trackConnInfoToString(const QVector<TrackConnectionInfo>& vec);

} // namespace utils

} // namespace ssplib

#endif // SVGTRA_H
