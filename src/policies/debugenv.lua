for name, s in pairs(server) do
  output = "whoami="..whoami.." server="..name..": <"  
  for metric, value in pairs(s) do
    output = output.." "..metric.."="..value
  end
  BAL_LOG(0, output, ">")
end
