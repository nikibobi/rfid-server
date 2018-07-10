using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace RFID.Controllers
{
    using Models;

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
        public async Task Create([FromBody] ViewModels.Event[] values)
        {
            var now = DateTime.Now;
            var maxOffset = values.Max(v => v.Offset);
            foreach (var value in values)
            {
                var time = now - TimeSpan.FromMilliseconds(maxOffset - value.Offset);
                await context.Events.AddAsync(new Event
                {
                    RFID = value.RFID,
                    State = value.State,
                    Time = time
                });
            }
            await context.SaveChangesAsync();
        }
    }
}
