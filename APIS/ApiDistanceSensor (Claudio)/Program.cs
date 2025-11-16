var builder = WebApplication.CreateBuilder(args);

// CORS - permitir todo
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowAll", policy =>
    {
        policy.AllowAnyOrigin()
              .AllowAnyHeader()
              .AllowAnyMethod();
    });
});

// Add controllers
builder.Services.AddControllers();

// Swagger
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

var app = builder.Build();

// Activar CORS
app.UseCors("AllowAll");

// Swagger
app.UseSwagger();
app.UseSwaggerUI();

// Controllers
app.MapControllers();

app.Run();
