#!/bin/bash
set -e 

echo "[1/4] Instalando dependencias del sistema..."
sudo apt-get update
sudo apt-get install -y \
  cmake g++ qt6-base-dev qt6-multimedia-dev \
  libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev \
  libwavpack1 libwavpack-dev libfreetype-dev \
  libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev \
  libyaml-cpp-dev

echo "[2/4] Compilando proyecto y corriendo tests..."

sudo rm -rf /var/$TP_NAME/assets

sudo mkdir -p /usr/bin
sudo mkdir -p /var/$TP_NAME/assets
sudo mkdir -p /etc/$TP_NAME
sudo mkdir -p /usr/share/applications
sudo mkdir -p /usr/share/pixmaps
sudo mkdir -p /var/$TP_NAME/editor/maps
sudo mkdir -p /var/$TP_NAME/server/available_maps

sudo rm -rf /var/$TP_NAME/editor/maps/*
sudo rm -rf /var/$TP_NAME/server/available_maps/*

sudo chmod 777 /var/$TP_NAME/editor/maps
sudo chmod 777 /var/$TP_NAME/server/available_maps


echo "[3/4] Instalando archivos..."

echo "[3.1] Creando accesos de escritorio..."

echo "[3.2] Copiando accesos al escritorio del usuario actual..."

DESKTOP_DIR="$(eval echo ~$SUDO_USER)/Desktop"

mkdir -p "$DESKTOP_DIR"

cp /usr/share/applications/$TP_NAME-*.desktop "$DESKTOP_DIR/"
chmod +x "$DESKTOP_DIR/$TP_NAME-"*.desktop

echo "[3.3] Si ves 'Untrusted Desktop File', hacé click derecho en los íconos del escritorio y seleccioná 'Permitir lanzamiento'."

echo "[4/4] Instalación exitosa."