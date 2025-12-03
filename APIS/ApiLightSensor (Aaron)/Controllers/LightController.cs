using Microsoft.AspNetCore.Mvc;
using ApiLightSensor.Models;
using MySql.Data.MySqlClient;

namespace ApiLightSensor.Controllers;

[ApiController]
[Route("[controller]")]
public class LightController : ControllerBase // constructor
{
    private readonly IConfiguration _config;

    public LightController(IConfiguration config)  
    {
        _config = config;
    }

    [HttpPost]
    public IActionResult PostLight([FromBody] LightReading reading) 
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr); // habre la conexion con mysql
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO light_reading (id_device, timestamp, value)
            VALUES (@dev, NOW(3), @val) 
        ", conn); // alias 

        cmd.Parameters.AddWithValue("@dev", reading.IdDevice);
        cmd.Parameters.AddWithValue("@val", (object?)reading.Value ?? DBNull.Value);

        cmd.ExecuteNonQuery();

        return Ok();
    }
}
