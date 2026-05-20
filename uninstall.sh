#!/bin/bash
set -e

TP_NAME="ArgentumOnline"

echo "[1/3] Eliminando accesos directos del escritorio..."

for tipo in editor client server; do
  FILE="/usr/share/applications/$TP_NAME-$tipo.desktop"
  DESKTOP_FILE="/home/$SUDO_USER/Desktop/$TP_NAME-$tipo.desktop"
  
  sudo rm -f "$FILE"
  sudo rm -f "$DESKTOP_FILE"
done

echo "[2/3] Eliminando archivos instalados del sistema..."

sudo rm -f /usr/bin/$TP_NAME-client
sudo rm -f /usr/bin/$TP_NAME-server
sudo rm -f /usr/bin/$TP_NAME-editor

sudo rm -rf /var/$TP_NAME
sudo rm -rf /etc/$TP_NAME

sudo rm -f /usr/share/pixmaps/$TP_NAME-*.png

echo "[3/3] Desinstalación exitosa."