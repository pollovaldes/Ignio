using System;
using System.Collections.Generic;

namespace SistemaIoT.Model
{
	// ============================================
	// DHT11 Reading (Temperature + Humidity combined)
	// ============================================
	public class Dht11Reading
	{
		public long Id { get; set; }
		public int IdDevice { get; set; }
		public DateTime Timestamp { get; set; }
		public decimal? Temperature { get; set; }
		public decimal? Humidity { get; set; }
	}

	// ============================================
	// Smoke Reading
	// ============================================
	public class SmokeReading
	{
		public long Id { get; set; }
		public int IdDevice { get; set; }
		public DateTime Timestamp { get; set; }
		public int? Value { get; set; }
	}

	// ============================================
	// PIR Reading
	// ============================================
	public class PirReading
	{
		public long Id { get; set; }
		public int IdDevice { get; set; }
		public DateTime Timestamp { get; set; }
		public bool? Motion { get; set; }
	}

	// ============================================
	// Distance Reading
	// ============================================
	public class DistanceReading
	{
		public long Id { get; set; }
		public int IdDevice { get; set; }
		public DateTime Timestamp { get; set; }
		public decimal? DistanceCm { get; set; }
	}

	// ============================================
	// Button Reading
	// ============================================
	public class ButtonReading
	{
		public long Id { get; set; }
		public int IdDevice { get; set; }
		public DateTime Timestamp { get; set; }
		public bool? Pressed { get; set; }
	}

	// ============================================
	// Alert
	// ============================================
	public class Alert
	{
		public string AlertUuid { get; set; }
		public int IdDevice { get; set; }
		public DateTime TimestampStarted { get; set; }
		public DateTime? TimestampEnded { get; set; }
		public bool? IsReal { get; set; }
		public int? NumSensorsTriggered { get; set; }
		public bool Responded { get; set; }
		public int? ResponseTimeSeconds { get; set; }
		public string AlertType { get; set; }  // "automatic" o "manual"
	}

	// ============================================
	// Alert Create Request (para POST)
	// ============================================
	public class AlertCreateRequest
	{
		public string AlertUuid { get; set; }
		public int IdDevice { get; set; }
		public DateTime TimestampStarted { get; set; }
		public int? NumSensorsTriggered { get; set; }
		public string AlertType { get; set; }  // "automatic" o "manual"
	}

	// ============================================
	// Alert Update Request (para PUT)
	// ============================================
	public class AlertUpdateRequest
	{
		public DateTime TimestampEnded { get; set; }
		public bool IsReal { get; set; }
		public int ResponseTimeSeconds { get; set; }
	}

	// ============================================
	// Unified Sensor Reading (para GET /readings/since)
	// ============================================
	public class UnifiedSensorReading
	{
		public string SensorType { get; set; }  // "dht11", "smoke", "pir", "distance", "button"
		public long Id { get; set; }
		public int IdDevice { get; set; }
		public DateTime Timestamp { get; set; }
		public object Value { get; set; }  // El valor puede ser decimal, int, bool, etc.
	}
}