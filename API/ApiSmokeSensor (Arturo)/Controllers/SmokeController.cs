using Microsoft.AspNetCore.Mvc;
using MySql.Data.MySqlClient;
using ApiSmokeSensor.Model;

namespace ApiSmokeSensor.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class SmokeController : ControllerBase
    {
        private readonly IConfiguration _configuration;

        public SmokeController(IConfiguration configuration)
        {
            _configuration = configuration;
        }

        [HttpPost]
        public IActionResult Post([FromBody] SmokeReading reading)
        {
            string connString = _configuration.GetConnectionString("DefaultConnection");

            using var conn = new MySqlConnection(connString);
            conn.Open();

            using var cmd = conn.CreateCommand();
            cmd.CommandText = @"INSERT INTO smoke_reading 
                                (id_device, timestamp, value) 
                                VALUES (@ID, NOW(3), @VAL)";

            cmd.Parameters.AddWithValue("@ID", reading.IdDevice);
            cmd.Parameters.AddWithValue("@VAL", reading.Value);

            cmd.ExecuteNonQuery();

            return Ok(new { message = "Smoke reading saved" });
        }
    }
}
