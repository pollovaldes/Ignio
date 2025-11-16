using Microsoft.AspNetCore.Mvc;
using ApiPirSensor.Models;
using MySql.Data.MySqlClient;

namespace ApiPirSensor.Controllers;

[ApiController]
[Route("[controller]")]
public class PirController : ControllerBase
{
    private readonly IConfiguration _config;

    public PirController(IConfiguration config)
    {
        _config = config;
    }

    [HttpPost]
    public IActionResult PostPir([FromBody] PirReading reading)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO pir_reading (id_device, timestamp, motion)
            VALUES (@dev, NOW(3), @mot)
        ", conn);

        cmd.Parameters.AddWithValue("@dev", reading.IdDevice);
        cmd.Parameters.AddWithValue("@mot", (object?)reading.Motion ?? DBNull.Value);

        cmd.ExecuteNonQuery();

        return Ok();
    }
}
