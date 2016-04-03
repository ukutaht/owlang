def mccarthy(n)
  if n > 100
    n - 10
  else
    mccarthy(mccarthy(n + 11))
  end
end

p mccarthy(90)
