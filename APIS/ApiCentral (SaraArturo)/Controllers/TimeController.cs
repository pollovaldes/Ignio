using Microsoft.AspNetCore.Mvc;
using ApiCentral.Models;

namespace ApiCentral.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class TimeController : ControllerBase
    {
        // devuelve la hora exacta del servidor con milisegundos
        [HttpGet]
        public ActionResult<TimeResponse> GetServerTime()
        {
            var response = new TimeResponse
            {
                // Formato ISO 8601 con milisegundos
                Timestamp = DateTime.UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fffZ")
            };

            return Ok(response);
        }
    }
}
