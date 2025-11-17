using Microsoft.AspNetCore.Mvc;
using MySql.Data.MySqlClient;
using ApiSmokeSensor.Model;

namespace ApiSmokeSensor.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class PirController : ControllerBase
    {
        private readonly IConfiguration _configuration;

        public PirController(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        [HttpPost]
        public IActionResult Post([FromBody] PirReading reading)
        {
            string connString = _configuration.GetConnectionString("DefaultConnection");

            using var conn = new MySqlConnection(connString);
            conn.Open();

            using var cmd = conn.CreateCommand();
            cmd.CommandText = @"INSERT INTO pir_reading 
                                (id_device, timestamp, duration_seconds, event_number) 
                                VALUES (@ID, NOW(3), @DURATION, @EVENT_NUM)";

            cmd.Parameters.AddWithValue("@ID", reading.IdDevice);
            cmd.Parameters.AddWithValue("@DURATION", reading.DurationSeconds ?? (object)DBNull.Value);
            cmd.Parameters.AddWithValue("@EVENT_NUM", reading.EventNumber ?? (object)DBNull.Value);

            cmd.ExecuteNonQuery();

            return Ok(new { message = "PIR event saved", eventNumber = reading.EventNumber });
        }
    }
}