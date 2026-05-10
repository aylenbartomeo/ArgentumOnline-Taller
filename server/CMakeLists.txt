target_sources(argentum_online_server
    PRIVATE
        main.cpp
        # Entidades principales
        src/model/entities/player.cpp
        src/model/entities/monster.cpp
        src/model/entities/citizen.cpp
        
        # Sistema de ítems
        src/model/items/Armor.cpp
        src/model/items/Weapon.cpp
        
        # Nota: Las interfaces (.h) y utilidades puras de encabezado 
        # no se listan aquí si no tienen un archivo .cpp asociado.
    )

# Esto permite que el compilador encuentre los headers dentro de src/
# sin tener que usar rutas relativas largas en los #include
target_include_directories(argentum_online_server 
    PRIVATE 
        src/model
        src/model/entities
        src/model/items
    )
