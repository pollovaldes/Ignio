using Microsoft.AspNetCore.Mvc;
using ApiDht11Sensor.Models;
using MySql.Data.MySqlClient;

namespace ApiDht11Sensor.Controllers;

[ApiController]
[Route("[controller]")]
public class Dht11Controller : ControllerBase
{
    private readonly IConfiguration _config;

    public Dht11Controller(IConfiguration config)
    {
        _config = config;
    }

    [HttpPost]
    public IActionResult PostDht11([FromBody] Dht11Reading reading)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO dht11_reading 
            (id_device, timestamp, temperature, humidity)
            VALUES (@dev, NOW(3), @temp, @hum)
        ", conn);

        cmd.Parameters.AddWithValue("@dev", reading.IdDevice);
        cmd.Parameters.AddWithValue("@temp", (object?)reading.Temperature ?? DBNull.Value);
        cmd.Parameters.AddWithValue("@hum", (object?)reading.Humidity ?? DBNull.Value);

        cmd.ExecuteNonQuery();

        return Ok();
    }
}
