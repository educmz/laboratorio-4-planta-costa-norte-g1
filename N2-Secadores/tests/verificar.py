"""Ejecutar con python3 tests/verificar.py. Requiere un compilador C++."""
import json
import os
import re
import subprocess
import tempfile
import unittest
from pathlib import Path

NODO = Path(__file__).resolve().parents[1]
SKETCH = (NODO / "sketch.ino").read_text()


class Contrato(unittest.TestCase):
    def test_circuito(self):
        circuito = json.loads((NODO / "diagram.json").read_text())
        cables = {tuple(c[:2]) for c in circuito["connections"]}
        for origen, nombre in [("gas:AOUT", "GAS"), ("dht:SDA", "DHT"),
                               ("baliza:R", "R"), ("baliza:G", "G"), ("baliza:B", "B")]:
            pin = re.search(rf"PIN_{nombre}\s*=\s*(\d+)", SKETCH)[1]
            self.assertIn((origen, "esp:" + pin), cables)
        self.assertIn(("baliza:COM", "esp:GND.2"), cables)

    def test_calibracion_y_salidas_digitales(self):
        # Compila las funciones copiadas literalmente del sketch con una salida GPIO de ensayo.
        patron = r"(?:const (?:int|float) (?:UMBRAL_\w+|NUM_PUNTOS|PPM_REF|ADC_REF)\b[^;]*;|(?:float convertirGas|void actualizarBaliza)\([^)]*\)\s*\{.*?^\})"
        funciones = re.findall(patron, SKETCH, re.S | re.M)
        cpp = """
#include <cassert>
#include <cmath>
static int rgb;
void baliza(int r,int g,int b){rgb=4*r+2*g+b;}
""" + "\n".join(funciones) + """
int main(){
  int adc[]={3337,3635,3762,3811,3839,3861,3916};
  int ppm[]={100,400,1000,1500,2000,2500,5000};
  for(int i=0;i<7;i++) assert(convertirGas(adc[i])==ppm[i]);
  assert(std::isnan(convertirGas(3336)) && std::isnan(convertirGas(3917)));
  assert(std::abs(convertirGas(3628)-393.0)<0.1);
  for(int i=3338;i<=3916;i++) assert(convertirGas(i)>convertirGas(i-1));
  actualizarBaliza(1500); assert(rgb==2);
  actualizarBaliza(1500.1); assert(rgb==6);
  actualizarBaliza(2000); assert(rgb==6);
  actualizarBaliza(2000.1); assert(rgb==4);
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            binary = Path(tmp) / "ensayo"
            subprocess.run([os.environ.get("CXX", "c++"), "-x", "c++", "-", "-o", binary],
                           input=cpp, text=True, check=True)
            subprocess.run([binary], check=True)


if __name__ == "__main__":
    unittest.main(verbosity=2)
