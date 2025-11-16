namespace ApiCentral.Models;

public class AlertCreateRequest
{
    public string AlertUuid { get; set; } = default!;
    public int IdDevice { get; set; }
    public DateTime TimestampStarted { get; set; }
    public int NumSensorsTriggered { get; set; }
    public string AlertType { get; set; } = default!;
}
