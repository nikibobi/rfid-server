using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace RFID.ViewModels
{
    using Models;

    public class Event
    {
        public string RFID { get; set; }
        public uint Offset { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public State State { get; set; }
    }
}
