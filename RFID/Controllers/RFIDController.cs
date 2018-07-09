using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;

namespace RFID.Controllers
{
    using Models;
    using System.Linq;

    [Route("[controller]")]
    public class RFIDController : Controller
    {
        private readonly RFIDContext context;

        public RFIDController(RFIDContext context)
        {
            this.context = context;
        }

        // GET /rfid
        [HttpGet]
        public async Task<IEnumerable<Event>> List()
        {
            return await context.Events.ToListAsync();
        }

        // GET /rfid/latest
        [HttpGet("latest")]
        public async Task<Event> Latest()
        {
            return await context.Events
                .OrderByDescending(e => e.Time)
                .FirstOrDefaultAsync();
        }

        // POST /rfid
        [HttpPost]
        public async Task Create([FromBody]Event[] values)
        {
            await context.Events.AddRangeAsync(values);
            await context.SaveChangesAsync();
        }
    }
}
