using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace RFID.Models
{
    public class Event
    {
        public Guid Id { get; set; }
        public string RFID { get; set; }
        public DateTime Time { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public State State { get; set; }
    }
}
