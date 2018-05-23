using Microsoft.EntityFrameworkCore;

namespace RFID.Models
{
    public class RFIDContext : DbContext
    {
        public RFIDContext(DbContextOptions<RFIDContext> options)
            : base(options)
        {
            Database.Migrate();
        }

        public DbSet<Event> Events { get; set; }

        protected override void OnModelCreating(ModelBuilder builder)
        {
            base.OnModelCreating(builder);

            builder.Entity<Event>(model =>
            {
                model.Property(e => e.Id).ValueGeneratedOnAdd();

                model.Property(e => e.RFID).IsRequired();

                model.Property(e => e.Time).HasDefaultValueSql("GETDATE()");
            });
        }
    }
}
