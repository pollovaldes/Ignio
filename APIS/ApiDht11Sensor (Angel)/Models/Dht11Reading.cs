namespace ApiDht11Sensor.Models;

public class Dht11Reading
{
    public int IdDevice { get; set; }
    public decimal? Temperature { get; set; }
    public decimal? Humidity { get; set; }
}
