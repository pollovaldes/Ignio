using MySql.Data.MySqlClient;

namespace ApiCentral.Services
{
    public class DataCleanupService : BackgroundService
    {
        private readonly IConfiguration _config;
        private readonly TimeSpan cleanupInterval = TimeSpan.FromMinutes(5);

        public DataCleanupService(IConfiguration config)
        {
            _config = config;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                try
                {
                    CleanTables();
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[CLEANUP ERROR] {ex.Message}");
                }

                await Task.Delay(cleanupInterval, stoppingToken);
            }
        }

        private void CleanTables()
        {
            string connStr = _config.GetConnectionString("DefaultConnection");

            using var conn = new MySqlConnection(connStr);
            conn.Open();

            string[] queries =
            {
                "DELETE FROM dht11_readin WHERE timestamp < NOW(3) - INTERVAL 15 MINUTE;",
                "DELETE FROM smoke_readin WHERE timestamp < NOW(3) - INTERVAL 10 MINUTE;",
                "DELETE FROM light_readin WHERE timestamp < NOW(3) - INTERVAL 10 MINUTE;",
                "DELETE FROM distance_readin WHERE timestamp < NOW(3) - INTERVAL 5 MINUTE;",
                "DELETE FROM pir_readig WHERE timestamp < NOW(3) - INTERVAL 5 MINUTE;",
                "DELETE FROM button_readin WHERE timestamp < NOW(3) - INTERVAL 20 MINUTE;"
            };

            foreach (string q in queries)
            {
                using var cmd = new MySqlCommand(q, conn);
                cmd.ExecuteNonQuery();
            }

            Console.WriteLine("[CLEANUP] Tablas limpiadas correctamente");
        }
    }
}
