#ifndef NODEFINDEREDITINGMODES_H
#define NODEFINDEREDITINGMODES_H

enum class ProgramMode
{
    NoMode = 0,
    SVGCreationMode,
    SVGMappingMode,
    NModes
};

//Editing modes for SVGMapping
enum class EditingModes
{
    NoSVGLoaded = 0,
    NoEditing,
    LabelEditing,
    StationTrackEditing,
    TrackPathEditing,
    SplitElement,
    NModes
};

enum class EditingSubModes
{
    NotEditingCurrentItem = 0,
    RemovingSubElement,
    AddingSubElement,
    DoSplitItem,
    NSubModes
};

#endif // NODEFINDEREDITINGMODES_H
