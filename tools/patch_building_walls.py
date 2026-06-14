#!/usr/bin/env python3
import json

MAP = "maps/defaultMap.json"

GRASS = 108
STONE = 17
DIRT = 106

BUILDINGS = {
    201: {"w": 15, "h": 18, "back": 6, "door": 1, "floor": 8, "rebuild": False, "clear_w": 15},
    202: {"w": 15, "h": 11, "back": 4, "door": 1, "floor": 5, "rebuild": True, "clear_w": 20},
}

SAFE = (1, 5, 36, 29)  # recuadro relativo al origen del stampeo: dx, dy, w, h


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


def city_ground(c, r):
    if not (0 <= c < 44 and 0 <= r < 34):
        return None
    sx, sy, sw, sh = SAFE
    if not (sx <= c < sx + sw and sy <= r < sy + sh):
        return GRASS
    v = DIRT
    if 2 <= c <= 16 and 5 <= r <= 22:
        v = STONE
    if 20 <= c <= 34 and 8 <= r <= 18:
        v = STONE
    if r == 23 and 9 <= c <= 27:
        v = STONE
    if c == 27 and 18 <= r <= 23:
        v = STONE
    if c == 9 and 22 <= r <= 23:
        v = STONE
    if c == 20 and 23 <= r <= 31:
        v = STONE
    return v


def main():
    with open(MAP) as f:
        m = json.load(f)

    decoration = m["decoration"]
    indoor = m["indoor"]
    ground = m["ground"]

    anchors = []
    banks = []
    for ay, rowvals in enumerate(decoration):
        for ax, v in enumerate(rowvals):
            s = BUILDINGS.get(v)
            if s:
                anchors.append((ax, ay, s))
            if v == 202:
                banks.append((ax, ay))

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

    sx, sy, sw, sh = SAFE
    for (bx, by) in banks:
        ox, oy = bx - 20, by - 18
        for r in range(34):
            for c in range(44):
                g = city_ground(c, r)
                if g is not None:
                    ground[oy + r][ox + c] = g
        for z in m["safeZones"]:
            if z["x"] <= bx < z["x"] + z["width"] and z["y"] <= by < z["y"] + z["height"]:
                z["x"], z["y"], z["width"], z["height"] = ox + sx, oy + sy, sw, sh
                break

    with open(MAP, "w") as f:
        f.write(json.dumps(m, indent=4, sort_keys=True))

    print("obstacles", len(m["obstacles"]), "added", len(added), "moved", moved, "cities", len(banks))


if __name__ == "__main__":
    main()
