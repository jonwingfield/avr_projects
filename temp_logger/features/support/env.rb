require 'rubygems'
require 'pty'
require 'webmock/cucumber'
require './lib/application'

# Create a pseudo-TTY port to communicate with the app through
# see man pty for more info
master,slave = PTY.open
sensor_path = slave.path
MockSensor = master

Thread.new do
	Application = WeatherIntelligence.new(sensor_path)
	Application.gather
end

