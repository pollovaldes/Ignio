using Microsoft.AspNetCore.Mvc;
using ApiCentral.Models;
using MySql.Data.MySqlClient;

namespace ApiCentral.Controllers;

[ApiController]
[Route("[controller]")]
public class AlertController : ControllerBase
{
    private readonly IConfiguration _config;

    public AlertController(IConfiguration cfg)
    {
        _config = cfg;
    }

    [HttpPost]
    public IActionResult CreateAlert([FromBody] AlertCreateRequest req)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            INSERT INTO alert (
                alert_uuid, 
                id_device, 
                timestamp_started, 
                timestamp_ended,
                is_real,
                num_sensors_triggered,
                responded,
                response_time_seconds,
                alert_type
            ) VALUES (
                @uuid, 
                @dev, 
                @start, 
                NULL,
                NULL,
                @num,
                FALSE,
                NULL,
                @type
            )
        ", conn);

        cmd.Parameters.AddWithValue("@uuid", req.AlertUuid);
        cmd.Parameters.AddWithValue("@dev", req.IdDevice);
        cmd.Parameters.AddWithValue("@start", req.TimestampStarted);
        cmd.Parameters.AddWithValue("@num", req.NumSensorsTriggered);
        cmd.Parameters.AddWithValue("@type", req.AlertType);

        cmd.ExecuteNonQuery();

        return Ok();
    }

    [HttpPut("{uuid}")]
    public IActionResult EndAlert(string uuid, [FromBody] AlertUpdateRequest req)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        using var cmd = new MySqlCommand(@"
            UPDATE alert
            SET 
                timestamp_ended = @end,
                is_real = @real,
                responded = TRUE,
                response_time_seconds = @seconds
            WHERE alert_uuid = @uuid
        ", conn);

        cmd.Parameters.AddWithValue("@end", req.TimestampEnded);
        cmd.Parameters.AddWithValue("@real", req.IsReal);
        cmd.Parameters.AddWithValue("@seconds", req.ResponseTimeSeconds);
        cmd.Parameters.AddWithValue("@uuid", uuid);

        cmd.ExecuteNonQuery();

        return Ok();
    }
}
