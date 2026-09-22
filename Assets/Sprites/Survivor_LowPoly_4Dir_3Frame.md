# Survivor low-poly sprite sheet

- Source: AI-generated image (original sprite sheet)
- Runtime asset: `Survivor_LowPoly_4Dir_3Frame_Source.png`
- Layout: 3 columns x 4 rows (12 views)
- Rows: front, back, left profile, right profile
- Columns: animation pose 1, pose 2, pose 3
- Intended runtime sample: 16-color indexed sprite, 24 x 24 px per frame
- Engine data: `NoWayDown/Renderer/CharacterSpriteData.h`

The checkerboard shown in the source is an opaque preview background. It should be
removed during the indexed-sprite conversion. The converted frames are now used
by the console renderer, with the original hard-coded silhouette retained in
the source as a fallback reference.
