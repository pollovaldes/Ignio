namespace ApiCentral.Models;

public class UnifiedSensorReading
{
    public string SensorType { get; set; } = default!;
    public long Id { get; set; }
    public int IdDevice { get; set; }
    public DateTime Timestamp { get; set; }
    public object? Value { get; set; }
}
