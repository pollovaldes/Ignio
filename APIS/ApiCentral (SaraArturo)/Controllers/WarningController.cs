using Microsoft.AspNetCore.Mvc;
using ApiCentral.Models;
using MySql.Data.MySqlClient;

namespace ApiCentral.Controllers;

[ApiController]
[Route("[controller]")]
public class WarningController : ControllerBase
{
    private readonly IConfiguration _config;

    public WarningController(IConfiguration cfg)
    {
        _config = cfg;
    }

    [HttpPost]
    public IActionResult PostWarning([FromBody] WarningEvent warning)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO warning_event 
            (id_device, timestamp, sensor_type, message)
            VALUES (@dev, NOW(3), @type, @msg)
        ", conn);

        cmd.Parameters.AddWithValue("@dev", warning.IdDevice);
        cmd.Parameters.AddWithValue("@type", warning.SensorType);
        cmd.Parameters.AddWithValue("@msg", warning.Message);

        cmd.ExecuteNonQuery();

        return Ok();
    }
}
