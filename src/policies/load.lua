for _, s in pairs(server) do
  s['load'] = s['metric1'] * (2*s['metric2'])
end
