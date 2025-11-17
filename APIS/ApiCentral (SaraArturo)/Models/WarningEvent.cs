namespace ApiCentral.Models;

public class WarningEvent
{
    public int IdDevice { get; set; }
    public string WarningType { get; set; } = default!;  // 'http_get_failed', 'http_post_failed', etc
    public string? HttpMethod { get; set; }               // GET, POST, PUT
    public string? HttpEndpoint { get; set; }             // /Readings/since, /Alert, etc
    public string Message { get; set; } = default!;       // Mensaje descriptivo
}