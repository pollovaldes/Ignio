using Microsoft.AspNetCore.Mvc;
using MySql.Data.MySqlClient;
using ApiCentral.Models;

namespace ApiCentral.Controllers;

[ApiController]
[Route("[controller]")]
public class TestController : ControllerBase
{
    private readonly IConfiguration _config;

    public TestController(IConfiguration cfg)
    {
        _config = cfg;
    }

    // Este endpoint solo sirve para pruebas
    // Devuelve los últimos 20 registros de todas las tablas de sensores
    [HttpGet("last20")]
    public IActionResult GetLast20()
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        var result = new Dictionary<string, List<UnifiedSensorReading>>();

        // Función auxiliar para no repetir código
        List<UnifiedSensorReading> Query(string sql, string type)
        {
            var list = new List<UnifiedSensorReading>();

            using var cmd = new MySqlCommand(sql, conn);
            using var reader = cmd.ExecuteReader();

            while (reader.Read())
            {
                list.Add(new UnifiedSensorReading
                {
                    SensorType = type,
                    Id = reader.GetInt64(0),
                    IdDevice = reader.GetInt32(1),
                    Timestamp = reader.GetDateTime(2),
                    Value = reader.IsDBNull(3) ? null : reader.GetValue(3)
                });
            }
            return list;
        }

        // Consultas de prueba
        result["smoke"] = Query("SELECT id, id_device, timestamp, value FROM smoke_reading ORDER BY id DESC LIMIT 20", "smoke");
        result["light"] = Query("SELECT id, id_device, timestamp, value FROM light_reading ORDER BY id DESC LIMIT 20", "light");
        result["dht11_temp"] = Query("SELECT id, id_device, timestamp, temperature FROM dht11_reading ORDER BY id DESC LIMIT 20", "temperature");
        result["dht11_humidity"] = Query("SELECT id, id_device, timestamp, humidity FROM dht11_reading ORDER BY id DESC LIMIT 20", "humidity");
        result["pir"] = Query("SELECT id, id_device, timestamp, motion FROM pir_reading ORDER BY id DESC LIMIT 20", "pir");
        result["distance"] = Query("SELECT id, id_device, timestamp, distance_cm FROM distance_reading ORDER BY id DESC LIMIT 20", "distance");

        return Ok(result);
    }
}
