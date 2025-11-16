namespace ApiCentral.Models;

public class AlertUpdateRequest
{
    public DateTime TimestampEnded { get; set; }
    public bool IsReal { get; set; }
    public int ResponseTimeSeconds { get; set; }
}
