#include <Arduino.h>
#include <LittleFS.h>

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println("Inicializando LittleFS...");

  // 1. Montar el sistema de archivos
  if (!LittleFS.begin())
  {
    Serial.println("❌ Error al montar LittleFS.");
    return;
  }

  Serial.println("✅ LittleFS montado correctamente.");

  // 2. Verificar espacio en memoria
  size_t total = LittleFS.totalBytes();
  size_t used = LittleFS.usedBytes();
  size_t free = total - used;

  Serial.println("📦 Estado de la memoria flash:");
  Serial.printf("  Total: %u bytes\n", total);
  Serial.printf("  Usado: %u bytes\n", used);
  Serial.printf("  Libre: %u bytes\n", free);

  // 3. Comprobar si existe el archivo
  const char *path = "/log.txt";

  if (LittleFS.exists(path))
  {
    Serial.printf("📁 El archivo %s existe.\n", path);

    // 4. Leer el archivo línea por línea
    File f = LittleFS.open(path, FILE_READ);
    if (!f)
    {
      Serial.println("⚠️ Error al abrir el archivo.");
      return;
    }

    Serial.printf("📄 Tamaño del archivo: %u bytes\n", f.size());
    Serial.println("📖 Contenido del archivo:");

    while (f.available())
    {
      String linea = f.readStringUntil('\n');
      Serial.println(linea);
    }

    f.close();
  }
  else
  {
    Serial.printf("❌ El archivo %s NO existe.\n", path);
  }
}

void loop()
{
  // No se necesita nada en el loop
}
