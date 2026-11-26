#pragma once
// Configurar únicamente en la copia privada secrets.h.
constexpr char CLAVE[] = "";
constexpr long AJUSTE_WOKWI_SEGUNDOS = 0;  // -20 solo para reproducir la sesión de Wokwi.
static_assert(sizeof(CLAVE) > 1, "Configurar la clave local del grupo");
