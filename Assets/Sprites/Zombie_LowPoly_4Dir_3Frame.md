# Zombie low-poly sprite sheet

- Source: `이미지 (15).png`
- Runtime asset: `Zombie_LowPoly_4Dir_3Frame_Source.png`
- Layout: 3 columns x 4 rows (12 views)
- Rows: front, back, left profile, right profile
- Columns: animation pose 1, pose 2, pose 3
- Source canvas: 1024 x 1024 px
- Intended runtime sample: 16-color indexed sprite, 24 x 24 px per frame
- Engine data: `NoWayDown/Renderer/CharacterSpriteData.h`

The current CraftEngine console renderer uses indexed C++ sprite data and does not
load PNG files directly. Keep this sheet as the art source; it must be converted
to the indexed frame data. The converted frames are now used by the console
renderer while the original hard-coded zombie shape remains as a fallback
reference in source.
