BAL_LOG(0, "Metrics from server "..whoami.."'s perspective")
for name, s in pairs(server) do
  output = " - cluster\t = server="..name..": <"
  for metric, value in pairs(s) do
    output = output.." "..metric.."="..value
  end
  BAL_LOG(0, output, " >")
end

output = " - timeseries = [ "
ts = timeseries()
for i=1,ts:size() do
  timestamp, id = ts:get(i)
  output = output.."("..timestamp..","..id..") "
end
BAL_LOG(0, output.."]")

WRstate(369)
BAL_LOG(0, "RDstate="..RDstate())

return false
