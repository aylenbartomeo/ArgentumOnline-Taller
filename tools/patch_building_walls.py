#!/usr/bin/env python3
import json

MAP = "maps/defaultMap.json"

GRASS = 108
STONE = 17

BUILDINGS = {
    201: {"w": 15, "h": 18, "back": 6, "door": 1, "floor": 8, "rebuild": False, "clear_w": 15},
    202: {"w": 15, "h": 11, "back": 4, "door": 1, "floor": 5, "rebuild": True, "clear_w": 20,
          "ground": 106},
}


def wall_cells(ax, ay, s):
    y0 = ay - s["h"] + 1
    x1 = ax + s["w"] - 1
    cx = ax + s["w"] // 2
    cells = set()
    for x in range(ax, x1 + 1):
        for r in range(y0, y0 + s["back"]):
            cells.add((x, r))
    for y in range(y0 + s["back"], ay + 1):
        cells.add((ax, y))
        cells.add((x1, y))
    for x in range(ax, x1 + 1):
        if not (cx - s["door"] <= x <= cx + s["door"]):
            cells.add((x, ay))
    return cells


def fix_ground(g, ax, ay, s):
    y0 = ay - s["h"] + 1
    x1 = ax + s["w"] - 1
    cx = ax + s["w"] // 2
    old_cx = ax + s["clear_w"] // 2
    for x in range(ax, x1 + 1):
        for y in range(y0, ay + 1):
            g[y][x] = s["ground"]
    for x in range(x1 + 1, ax + s["clear_w"]):
        for y in range(y0, ay + 1):
            g[y][x] = GRASS
    y = ay + 1
    steps = 0
    while steps < 8:
        if g[y][old_cx] != STONE:
            break
        if g[y][old_cx - 3] == STONE and g[y][old_cx + 3] == STONE:
            break
        for dx in range(-s["door"], s["door"] + 1):
            g[y][old_cx + dx] = GRASS
        for dx in range(-s["door"], s["door"] + 1):
            g[y][cx + dx] = STONE
        y += 1
        steps += 1


def main():
    with open(MAP) as f:
        m = json.load(f)

    decoration = m["decoration"]
    indoor = m["indoor"]
    ground = m["ground"]

    anchors = []
    for ay, rowvals in enumerate(decoration):
        for ax, v in enumerate(rowvals):
            s = BUILDINGS.get(v)
            if s:
                anchors.append((ax, ay, s))

    clear = set()
    walls = set()
    bands = []
    for (ax, ay, s) in anchors:
        y0 = ay - s["h"] + 1
        x1 = ax + s["w"] - 1
        walls |= wall_cells(ax, ay, s)
        bands.append((ax, x1, y0, ay, y0 + s["floor"]))
        if not s["rebuild"]:
            continue
        for x in range(ax, ax + s["clear_w"]):
            for y in range(y0, ay + 1):
                clear.add((x, y))
                indoor[y][x] = 0
        for x in range(ax, x1 + 1):
            for y in range(y0, ay + 1):
                indoor[y][x] = 1
        fix_ground(ground, ax, ay, s)

    kept = [o for o in m["obstacles"] if (o["x"], o["y"]) not in clear]
    kept_set = {(o["x"], o["y"]) for o in kept}
    added = [{"x": x, "y": y} for (x, y) in sorted(walls) if (x, y) not in kept_set]
    m["obstacles"] = kept + added
    obs_now = kept_set | {(c["x"], c["y"]) for c in added}

    moved = 0
    for npc in m["npcs"]:
        for (clo, chi, rlo, rhi, floor_row) in bands:
            if not (clo <= npc["x"] <= chi and rlo <= npc["y"] <= rhi):
                continue
            if (npc["x"], npc["y"]) not in obs_now:
                break
            npc["x"] = min(max(npc["x"], clo + 1), chi - 1)
            npc["y"] = floor_row
            moved += 1
            break

    with open(MAP, "w") as f:
        f.write(json.dumps(m, indent=4, sort_keys=True))

    print("obstacles:", len(m["obstacles"]), "| added", len(added), "| moved", moved, "npcs")


if __name__ == "__main__":
    main()
