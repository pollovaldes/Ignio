using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using SistemaIoT.Model;
using Microsoft.AspNetCore.Mvc;
using MySql.Data.MySqlClient;

namespace SistemaIoT.Controllers
{
    // ============================================
    // CONTROLLER: Temperatura
    // ============================================
    [Route("[controller]")]
    public class TemperaturaController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        // GET: temperatura
        [HttpGet]
        public List<LecturaTemperatura> Get()
        {
            List<LecturaTemperatura> lista = new List<LecturaTemperatura>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Temperatura ORDER BY fecha_hora DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaTemperatura item = new LecturaTemperatura();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Temperatura = Convert.ToDecimal(reader["temperatura"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        // GET: temperatura/dispositivo/1
        [HttpGet("dispositivo/{idDispositivo}")]
        public List<LecturaTemperatura> GetPorDispositivo(int idDispositivo)
        {
            List<LecturaTemperatura> lista = new List<LecturaTemperatura>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Temperatura WHERE id_dispositivo = @ID ORDER BY fecha_hora DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaTemperatura item = new LecturaTemperatura();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Temperatura = Convert.ToDecimal(reader["temperatura"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        // POST: temperatura
        [HttpPost]
        public void Post([FromBody] LecturaTemperatura lectura)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Lectura_Temperatura (id_dispositivo, fecha_hora, es_valida, temperatura) VALUES (@ID_DISP, NOW(), @ES_VALIDA, @TEMPERATURA)";
            cmd.Parameters.AddWithValue("@ID_DISP", lectura.IdDispositivo);
            cmd.Parameters.AddWithValue("@ES_VALIDA", lectura.EsValida);
            cmd.Parameters.AddWithValue("@TEMPERATURA", lectura.Temperatura);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: Humedad
    // ============================================
    [Route("[controller]")]
    public class HumedadController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        [HttpGet]
        public List<LecturaHumedad> Get()
        {
            List<LecturaHumedad> lista = new List<LecturaHumedad>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Humedad ORDER BY fecha_hora DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaHumedad item = new LecturaHumedad();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Humedad = Convert.ToDecimal(reader["humedad"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpGet("dispositivo/{idDispositivo}")]
        public List<LecturaHumedad> GetPorDispositivo(int idDispositivo)
        {
            List<LecturaHumedad> lista = new List<LecturaHumedad>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Humedad WHERE id_dispositivo = @ID ORDER BY fecha_hora DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaHumedad item = new LecturaHumedad();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Humedad = Convert.ToDecimal(reader["humedad"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpPost]
        public void Post([FromBody] LecturaHumedad lectura)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Lectura_Humedad (id_dispositivo, fecha_hora, es_valida, humedad) VALUES (@ID_DISP, NOW(), @ES_VALIDA, @HUMEDAD)";
            cmd.Parameters.AddWithValue("@ID_DISP", lectura.IdDispositivo);
            cmd.Parameters.AddWithValue("@ES_VALIDA", lectura.EsValida);
            cmd.Parameters.AddWithValue("@HUMEDAD", lectura.Humedad);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: Humo
    // ============================================
    [Route("[controller]")]
    public class HumoController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        [HttpGet]
        public List<LecturaHumo> Get()
        {
            List<LecturaHumo> lista = new List<LecturaHumo>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Humo ORDER BY fecha_hora DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaHumo item = new LecturaHumo();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Valor = Convert.ToInt32(reader["valor"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpGet("dispositivo/{idDispositivo}")]
        public List<LecturaHumo> GetPorDispositivo(int idDispositivo)
        {
            List<LecturaHumo> lista = new List<LecturaHumo>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Humo WHERE id_dispositivo = @ID ORDER BY fecha_hora DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaHumo item = new LecturaHumo();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Valor = Convert.ToInt32(reader["valor"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpPost]
        public void Post([FromBody] LecturaHumo lectura)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Lectura_Humo (id_dispositivo, fecha_hora, es_valida, valor) VALUES (@ID_DISP, NOW(), @ES_VALIDA, @VALOR)";
            cmd.Parameters.AddWithValue("@ID_DISP", lectura.IdDispositivo);
            cmd.Parameters.AddWithValue("@ES_VALIDA", lectura.EsValida);
            cmd.Parameters.AddWithValue("@VALOR", lectura.Valor);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: PIR
    // ============================================
    [Route("[controller]")]
    public class PIRController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        [HttpGet]
        public List<LecturaPIR> Get()
        {
            List<LecturaPIR> lista = new List<LecturaPIR>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_PIR ORDER BY fecha_hora DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaPIR item = new LecturaPIR();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Movimiento = Convert.ToBoolean(reader["movimiento"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpGet("dispositivo/{idDispositivo}")]
        public List<LecturaPIR> GetPorDispositivo(int idDispositivo)
        {
            List<LecturaPIR> lista = new List<LecturaPIR>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_PIR WHERE id_dispositivo = @ID ORDER BY fecha_hora DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaPIR item = new LecturaPIR();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Movimiento = Convert.ToBoolean(reader["movimiento"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpPost]
        public void Post([FromBody] LecturaPIR lectura)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Lectura_PIR (id_dispositivo, fecha_hora, es_valida, movimiento) VALUES (@ID_DISP, NOW(), @ES_VALIDA, @MOVIMIENTO)";
            cmd.Parameters.AddWithValue("@ID_DISP", lectura.IdDispositivo);
            cmd.Parameters.AddWithValue("@ES_VALIDA", lectura.EsValida);
            cmd.Parameters.AddWithValue("@MOVIMIENTO", lectura.Movimiento);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: Distancia
    // ============================================
    [Route("[controller]")]
    public class DistanciaController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        [HttpGet]
        public List<LecturaDistancia> Get()
        {
            List<LecturaDistancia> lista = new List<LecturaDistancia>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Distancia ORDER BY fecha_hora DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaDistancia item = new LecturaDistancia();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.DistanciaCm = Convert.ToDecimal(reader["distancia_cm"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpGet("dispositivo/{idDispositivo}")]
        public List<LecturaDistancia> GetPorDispositivo(int idDispositivo)
        {
            List<LecturaDistancia> lista = new List<LecturaDistancia>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Distancia WHERE id_dispositivo = @ID ORDER BY fecha_hora DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaDistancia item = new LecturaDistancia();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.DistanciaCm = Convert.ToDecimal(reader["distancia_cm"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpPost]
        public void Post([FromBody] LecturaDistancia lectura)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Lectura_Distancia (id_dispositivo, fecha_hora, es_valida, distancia_cm) VALUES (@ID_DISP, NOW(), @ES_VALIDA, @DISTANCIA)";
            cmd.Parameters.AddWithValue("@ID_DISP", lectura.IdDispositivo);
            cmd.Parameters.AddWithValue("@ES_VALIDA", lectura.EsValida);
            cmd.Parameters.AddWithValue("@DISTANCIA", lectura.DistanciaCm);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: Botón
    // ============================================
    [Route("[controller]")]
    public class BotonController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        [HttpGet]
        public List<LecturaBoton> Get()
        {
            List<LecturaBoton> lista = new List<LecturaBoton>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Boton ORDER BY fecha_hora DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaBoton item = new LecturaBoton();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Presionado = Convert.ToBoolean(reader["presionado"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpGet("dispositivo/{idDispositivo}")]
        public List<LecturaBoton> GetPorDispositivo(int idDispositivo)
        {
            List<LecturaBoton> lista = new List<LecturaBoton>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Lectura_Boton WHERE id_dispositivo = @ID ORDER BY fecha_hora DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    LecturaBoton item = new LecturaBoton();
                    item.IdLectura = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaHora = Convert.ToDateTime(reader["fecha_hora"]);
                    item.EsValida = Convert.ToBoolean(reader["es_valida"]);
                    item.Presionado = Convert.ToBoolean(reader["presionado"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpPost]
        public void Post([FromBody] LecturaBoton lectura)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Lectura_Boton (id_dispositivo, fecha_hora, es_valida, presionado) VALUES (@ID_DISP, NOW(), @ES_VALIDA, @PRESIONADO)";
            cmd.Parameters.AddWithValue("@ID_DISP", lectura.IdDispositivo);
            cmd.Parameters.AddWithValue("@ES_VALIDA", lectura.EsValida);
            cmd.Parameters.AddWithValue("@PRESIONADO", lectura.Presionado);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: Alertas
    // ============================================
    [Route("[controller]")]
    public class AlertaController : Controller
    {
        private string connectionString = "Server=127.0.0.1;Port=3306;Database=Ignio;Uid=root;password=;";

        [HttpGet]
        public List<Alerta> Get()
        {
            List<Alerta> lista = new List<Alerta>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Alerta ORDER BY fecha_generacion DESC LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    Alerta item = new Alerta();
                    item.IdAlerta = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaGeneracion = Convert.ToDateTime(reader["fecha_generacion"]);
                    item.FechaRespuesta = reader["fecha_respuesta"] == DBNull.Value ? null : Convert.ToDateTime(reader["fecha_respuesta"]);
                    item.EsReal = reader["es_real"] == DBNull.Value ? null : Convert.ToBoolean(reader["es_real"]);
                    item.DuroSegundos = reader["duro_segundos"] == DBNull.Value ? null : Convert.ToInt32(reader["duro_segundos"]);
                    item.NumeroSensores = reader["numero_sensores"] == DBNull.Value ? null : Convert.ToInt32(reader["numero_sensores"]);
                    item.Respondida = Convert.ToBoolean(reader["respondida"]);
                    item.TiempoRespuestaSegundos = reader["tiempo_respuesta_segundos"] == DBNull.Value ? null : Convert.ToInt32(reader["tiempo_respuesta_segundos"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpGet("dispositivo/{idDispositivo}")]
        public List<Alerta> GetPorDispositivo(int idDispositivo)
        {
            List<Alerta> lista = new List<Alerta>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "SELECT * FROM Alerta WHERE id_dispositivo = @ID ORDER BY fecha_generacion DESC LIMIT 100";
            cmd.Parameters.AddWithValue("@ID", idDispositivo);

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    Alerta item = new Alerta();
                    item.IdAlerta = Convert.ToInt64(reader["id"]);
                    item.IdDispositivo = Convert.ToInt32(reader["id_dispositivo"]);
                    item.FechaGeneracion = Convert.ToDateTime(reader["fecha_generacion"]);
                    item.FechaRespuesta = reader["fecha_respuesta"] == DBNull.Value ? null : Convert.ToDateTime(reader["fecha_respuesta"]);
                    item.EsReal = reader["es_real"] == DBNull.Value ? null : Convert.ToBoolean(reader["es_real"]);
                    item.DuroSegundos = reader["duro_segundos"] == DBNull.Value ? null : Convert.ToInt32(reader["duro_segundos"]);
                    item.NumeroSensores = reader["numero_sensores"] == DBNull.Value ? null : Convert.ToInt32(reader["numero_sensores"]);
                    item.Respondida = Convert.ToBoolean(reader["respondida"]);
                    item.TiempoRespuestaSegundos = reader["tiempo_respuesta_segundos"] == DBNull.Value ? null : Convert.ToInt32(reader["tiempo_respuesta_segundos"]);

                    lista.Add(item);
                }
            }

            conexion.Close();
            return lista;
        }

        [HttpPost]
        public void Post([FromBody] Alerta alerta)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = "INSERT INTO Alerta (id_dispositivo, fecha_generacion, numero_sensores, respondida) VALUES (@ID_DISP, NOW(), @NUM_SENSORES, @RESPONDIDA)";
            cmd.Parameters.AddWithValue("@ID_DISP", alerta.IdDispositivo);
            cmd.Parameters.AddWithValue("@NUM_SENSORES", alerta.NumeroSensores);
            cmd.Parameters.AddWithValue("@RESPONDIDA", alerta.Respondida);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }

        // PUT: alerta/1/responder
        [HttpPut("{id}/responder")]
        public void ResponderAlerta(long id)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"
                UPDATE Alerta 
                SET fecha_respuesta = NOW(), 
                    respondida = TRUE,
                    tiempo_respuesta_segundos = TIMESTAMPDIFF(SECOND, fecha_generacion, NOW())
                WHERE id = @ID
            ";
            cmd.Parameters.AddWithValue("@ID", id);

            cmd.Prepare();
            cmd.ExecuteNonQuery();

            conexion.Close();
        }
    }
}