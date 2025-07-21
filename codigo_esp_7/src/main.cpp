#include <Arduino.h>
#include <LittleFS.h>

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println("Inicializando LittleFS...");

  if (!LittleFS.begin())
  {
    Serial.println("❌ Error al montar LittleFS.");
    return;
  }

  Serial.println("✅ LittleFS montado correctamente.");

  const char *path = "/log.txt";

  if (!LittleFS.exists(path))
  {
    Serial.printf("❌ El archivo %s NO existe.\n", path);
    return;
  }

  File f = LittleFS.open(path, FILE_READ);
  if (!f)
  {
    Serial.println("⚠️ Error al abrir el archivo.");
    return;
  }

  Serial.printf("📄 Tamaño del archivo: %u bytes\n", f.size());
  Serial.println("📖 Contenido limpio del archivo (omitiendo líneas corruptas):");

  while (f.available())
  {
    String linea = f.readStringUntil('\n');
    linea.trim(); // Elimina espacios y saltos de línea

    // Si la línea está vacía, saltar
    if (linea.length() == 0)
      continue;

    // Intentamos ver si es un número entero (positivo o negativo)
    char *endPtr;
    long valor = strtol(linea.c_str(), &endPtr, 10);

    bool esNumeroEntero = (*endPtr == '\0');

    // Si es número, verificamos que esté en un rango válido
    if (esNumeroEntero)
    {
      if (valor >= 0 && valor <= 1000000)
      {
        Serial.println(valor);
      }
      else
      {
        // Saltamos la línea corrupta sin imprimir nada
      }
    }
    else
    {
      // No es un número → imprimir como texto normal
      Serial.println(linea);
    }
  }

  f.close();
}

void loop()
{
  // Nada aquí
}
