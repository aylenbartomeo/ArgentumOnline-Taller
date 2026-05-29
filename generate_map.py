import json
import os

width = 60
height = 60

# Creamos la grilla 60x60 llena de pasto (tile ID 0)
tiles = [[0 for _ in range(width)] for _ in range(height)]

# Spawn point central
spawn_x = 30
spawn_y = 30

# Definimos 3 ciudades
# 1. Ullathorpe (Centro)
# 2. Nix (Norte-Oeste)
# 3. Banderbill (Sur-Este)

safeZones = [
    { "name": "Ullathorpe", "x": 25, "y": 25, "width": 10, "height": 10 },
    { "name": "Nix", "x": 5, "y": 5, "width": 10, "height": 10 },
    { "name": "Banderbill", "x": 45, "y": 45, "width": 10, "height": 10 }
]

npcs = [
    # Ullathorpe NPCs
    { "type": "merchant", "x": 27, "y": 28 },
    { "type": "priest",   "x": 30, "y": 28 },
    { "type": "banker",   "x": 33, "y": 28 },
    
    # Nix NPCs
    { "type": "merchant", "x": 7, "y": 8 },
    { "type": "priest",   "x": 10, "y": 8 },
    { "type": "banker",   "x": 13, "y": 8 },

    # Banderbill NPCs
    { "type": "merchant", "x": 47, "y": 48 },
    { "type": "priest",   "x": 50, "y": 48 },
    { "type": "banker",   "x": 53, "y": 48 }
]

data = {
    "tileSize": 16,
    "tileset": "tilemap_packed.png",
    "tilesetCols": 12,
    "width": width,
    "height": height,
    "spawn": { "x": spawn_x, "y": spawn_y },
    "safeZones": safeZones,
    "npcs": npcs,
    "tiles": tiles
}

with open("maps/defaultMap.json", "w") as f:
    json.dump(data, f, indent=4)

print("Mapa de 60x60 con 3 ciudades generado exitosamente en maps/defaultMap.json")
