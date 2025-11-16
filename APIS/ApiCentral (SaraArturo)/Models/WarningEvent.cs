namespace ApiCentral.Models;

public class WarningEvent
{
    public int IdDevice { get; set; }
    public string SensorType { get; set; } = default!;
    public string Message { get; set; } = default!;
}
