require 'rubygems'
require 'pty'
require 'webmock/cucumber'
require 'mail'
require_relative '../../app'

# Create a pseudo-TTY port to communicate with the app through
# see man pty for more info
master,slave = PTY.open
sensor_path = slave.path
MockSensor = master

Thread.new do
  Mail.defaults do
    delivery_method :test
  end
  
  Application = WeatherIntelligence.new(sensor_path)
  Application.gather
end
