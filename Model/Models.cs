using System;

namespace SistemaIoT.Model
{
	// ============================================
	// Modelo: Lectura de Temperatura
	// ============================================
	public class LecturaTemperatura
	{
		public long IdLectura { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaHora { get; set; }
		public bool EsValida { get; set; }
		public decimal Temperatura { get; set; }

		public LecturaTemperatura()
		{
		}
	}

	// ============================================
	// Modelo: Lectura de Humedad
	// ============================================
	public class LecturaHumedad
	{
		public long IdLectura { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaHora { get; set; }
		public bool EsValida { get; set; }
		public decimal Humedad { get; set; }

		public LecturaHumedad()
		{
		}
	}

	// ============================================
	// Modelo: Lectura de Humo
	// ============================================
	public class LecturaHumo
	{
		public long IdLectura { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaHora { get; set; }
		public bool EsValida { get; set; }
		public int Valor { get; set; }

		public LecturaHumo()
		{
		}
	}

	// ============================================
	// Modelo: Lectura de PIR
	// ============================================
	public class LecturaPIR
	{
		public long IdLectura { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaHora { get; set; }
		public bool EsValida { get; set; }
		public bool Movimiento { get; set; }

		public LecturaPIR()
		{
		}
	}

	// ============================================
	// Modelo: Lectura de Distancia
	// ============================================
	public class LecturaDistancia
	{
		public long IdLectura { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaHora { get; set; }
		public bool EsValida { get; set; }
		public decimal DistanciaCm { get; set; }

		public LecturaDistancia()
		{
		}
	}

	// ============================================
	// Modelo: Lectura de Botón
	// ============================================
	public class LecturaBoton
	{
		public long IdLectura { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaHora { get; set; }
		public bool EsValida { get; set; }
		public bool Presionado { get; set; }

		public LecturaBoton()
		{
		}
	}

	// ============================================
	// Modelo: Alerta
	// ============================================
	public class Alerta
	{
		public long IdAlerta { get; set; }
		public int IdDispositivo { get; set; }
		public DateTime FechaGeneracion { get; set; }
		public DateTime? FechaRespuesta { get; set; }
		public bool? EsReal { get; set; }
		public int? DuroSegundos { get; set; }
		public int? NumeroSensores { get; set; }
		public bool Respondida { get; set; }
		public int? TiempoRespuestaSegundos { get; set; }

		public Alerta()
		{
		}
	}
}