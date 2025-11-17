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
        // Validar que warningType sea válido
        string[] validTypes = {
            "http_get_failed",
            "http_post_failed",
            "http_put_failed",
            "connection_lost",
            "sensor_anomaly",
            "motion_detected",
            "distance_anomaly"
        };

        if (!validTypes.Contains(warning.WarningType?.ToLower()))
        {
            return BadRequest(new { error = $"Invalid warningType: {warning.WarningType}" });
        }

        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO warning_event 
            (id_device, timestamp, warning_type, http_method, http_endpoint, message)
            VALUES (@dev, NOW(3), @type, @method, @endpoint, @msg)
        ", conn);

        cmd.Parameters.AddWithValue("@dev", warning.IdDevice);
        cmd.Parameters.AddWithValue("@type", warning.WarningType.ToLower());
        cmd.Parameters.AddWithValue("@method", warning.HttpMethod ?? (object)DBNull.Value);
        cmd.Parameters.AddWithValue("@endpoint", warning.HttpEndpoint ?? (object)DBNull.Value);
        cmd.Parameters.AddWithValue("@msg", warning.Message);

        try
        {
            cmd.ExecuteNonQuery();
            Console.WriteLine($"[WARNING] Device={warning.IdDevice}, Type={warning.WarningType}, Method={warning.HttpMethod}, Endpoint={warning.HttpEndpoint}, Message={warning.Message}");
            return Ok(new { success = true });
        }
        catch (MySqlException ex)
        {
            Console.WriteLine($"[ERROR] No se pudo insertar warning: {ex.Message}");
            return StatusCode(500, new { error = ex.Message });
        }
    }
}