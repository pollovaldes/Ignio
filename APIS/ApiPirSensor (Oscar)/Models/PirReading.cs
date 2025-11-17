namespace ApiSmokeSensor.Model
{
    public class PirReading
    {
        public int IdDevice { get; set; }
        public float? DurationSeconds { get; set; }
        public int? EventNumber { get; set; }
    }
}