require 'rubygems'
require 'pty'
require 'webmock/cucumber'
require './lib/application'

m,s = PTY.open
sensor_path = s.path
MockSensor = m

Thread.new do
	Application = WeatherIntelligence.new(sensor_path)
	Application.gather
end

