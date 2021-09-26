#ifndef NODEFINDEDITINGMODES_H
#define NODEFINDEDITINGMODES_H

//Editing mode
enum class EditingModes
{
    NoSVGLoaded = 0,
    NoEditing,
    LabelEditing,
    StationTrackEditing,
    TrackPathEditing,
    NModes
};

enum class EditingSubModes
{
    NotEditingCurrentItem = 0,
    RemovingSubElement,
    AddingSubElement,
    NSubModes
};

#endif // NODEFINDEDITINGMODES_H
