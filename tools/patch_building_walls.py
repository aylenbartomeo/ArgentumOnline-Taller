#!/usr/bin/env python3
import json

MAP = "maps/defaultMap.json"

BUILDINGS = {
    201: {"w": 15, "h": 18, "back": 6, "floor": 8},
    202: {"w": 20, "h": 11, "back": 4, "floor": 5},
}


def main():
    with open(MAP) as f:
        m = json.load(f)

    decoration = m["decoration"]

    existing = m["obstacles"]
    existing_set = {(o["x"], o["y"]) for o in existing}
    new_cells = []
    bands = []

    for ay, rowvals in enumerate(decoration):
        for ax, v in enumerate(rowvals):
            spec = BUILDINGS.get(v)
            if not spec:
                continue
            y0 = ay - (spec["h"] - 1)
            x1 = ax + spec["w"] - 1
            row_lo, row_hi = y0, y0 + spec["back"] - 1
            bands.append((ax, x1, row_lo, row_hi, y0 + spec["floor"]))
            for col in range(ax, x1 + 1):
                for r in range(row_lo, row_hi + 1):
                    if (col, r) not in existing_set:
                        existing_set.add((col, r))
                        new_cells.append({"x": col, "y": r})

    moved = 0
    for npc in m["npcs"]:
        for (clo, chi, rlo, rhi, floor_row) in bands:
            if clo <= npc["x"] <= chi and rlo <= npc["y"] <= rhi:
                npc["x"] = min(max(npc["x"], clo + 1), chi - 1)
                npc["y"] = floor_row
                moved += 1
                break

    m["obstacles"] = existing + new_cells

    with open(MAP, "w") as f:
        f.write(json.dumps(m, indent=4, sort_keys=True))

    print("added", len(new_cells), "back-wall obstacles, moved", moved, "npcs")


if __name__ == "__main__":
    main()
