require 'weather_history'

describe 'Weather History' do
	it 'should record a max temperature within the last 24 hours' do
		history = WeatherHistory.new

		history.log_event(WeatherReading.new(17.4, 55.3), DateTime.new(2001, 6, 4, 11, 10, 1))
		history.log_event(WeatherReading.new(18.2, 64.2), DateTime.new(2001, 6, 4, 11, 12, 1))
		history.log_event(WeatherReading.new(15.3, 43.7), DateTime.new(2001, 6, 4, 11, 15, 0))
		history.log_event(WeatherReading.new(17.9, 43.7), DateTime.new(2001, 6, 4, 22, 20, 0))

		history.max_temp.temp.c.should == 18.2
		history.max_temp.time.should == DateTime.new(2001, 6, 4, 11, 12, 1)
	end

	it "should record a min temperature within the last 24 hours" do
		history = WeatherHistory.new

		history.log_event(WeatherReading.new(17.4, 55.3), DateTime.new(2001, 6, 4, 11, 10, 1))
		history.log_event(WeatherReading.new(15.3, 43.7), DateTime.new(2001, 6, 4, 11, 15, 0))
		history.log_event(WeatherReading.new(18.2, 64.2), DateTime.new(2001, 6, 4, 11, 12, 1))

		history.min_temp.temp.c.should == 15.3
		history.min_temp.time.should == DateTime.new(2001, 6, 4, 11, 15, 0)
	end

	it "should publish a notification when the max or min temperature changes" do
		history = WeatherHistory.new
		notification = double('extremesChanged')
		notification.should_receive(:extremes_changed).with(history).twice
		history.add_notification notification

		history.log_event(WeatherReading.new(17.4, 55.3), DateTime.new(2001, 6, 4, 11, 10, 1))
		history.log_event(WeatherReading.new(18.2, 64.2), DateTime.new(2001, 6, 4, 11, 12, 1))
		history.log_event(WeatherReading.new(18.1, 64.9), DateTime.new(2001, 6, 4, 11, 12, 1))
	end
end


