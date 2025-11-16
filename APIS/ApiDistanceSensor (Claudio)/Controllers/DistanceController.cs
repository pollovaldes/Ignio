using Microsoft.AspNetCore.Mvc;
using ApiDistanceSensor.Models;
using MySql.Data.MySqlClient;

namespace ApiDistanceSensor.Controllers;

[ApiController]
[Route("[controller]")]
public class DistanceController : ControllerBase
{
    private readonly IConfiguration _config;

    public DistanceController(IConfiguration config)
    {
        _config = config;
    }

    [HttpPost]
    public IActionResult PostDistance([FromBody] DistanceReading reading)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO distance_reading (id_device, timestamp, distance_cm)
            VALUES (@dev, NOW(3), @dist)
        ", conn);

        cmd.Parameters.AddWithValue("@dev", reading.IdDevice);
        cmd.Parameters.AddWithValue("@dist", (object?)reading.DistanceCm ?? DBNull.Value);

        cmd.ExecuteNonQuery();

        return Ok();
    }
}
