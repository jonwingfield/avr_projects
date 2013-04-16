require 'weather_monitors'

shared_examples "A Monitor" do
	let(:fake_clock) { double("fake") }

	def send_notifications
		monitor.extremes_changed(notification_trigger) 
	end

	it "should send a notification on every violation when no time period specified" do
		notification.should_receive(:call).with(any_args()).twice
		2.times { send_notifications }
	end

	it "should allow notifications to be sent only once in an hour" do
		fake_clock.stub(:now).and_return(Time.now, Time.now, Time.now + (60*60))

		notification.should_receive(:call).with(any_args()).twice
		monitor.send_notifications_only :hourly
		monitor.timer.clock = fake_clock
		3.times { send_notifications }
	end

	it "should allow notifications to be sent only once in a day" do
		fake_clock.stub(:now).and_return(Time.now, Time.now, Time.now + (60*60*24))

		notification.should_receive(:call).with(any_args()).twice
		monitor.send_notifications_only :daily
		monitor.timer.clock = fake_clock
		3.times { send_notifications }
	end
end

describe 'Temperature Range Monitor' do
	let(:notification) { double("notification") }

	let(:monitor) do 
		monitor = TemperatureRangeMonitor.new(:max_variation => 9)
		monitor.when_range_exceeded(notification)
		monitor
	end

	let(:notification_trigger) do
		history = double("history")
		history.stub(:max_temp => HistoryRecord.new('30C', DateTime.now))
		history.stub(:min_temp => HistoryRecord.new('20C', DateTime.now))
		history
	end

	let(:pass) do
		history = double("history")
		history.stub(:max_temp => HistoryRecord.new('30C', DateTime.now))
		history.stub(:min_temp => HistoryRecord.new('21C', DateTime.now))
		history
	end

	it_behaves_like "A Monitor"

	it "should notify if the range exceeds the limit" do
		notification.should_receive(:call).with(
			"Maximum temperature range of 9.0C exceeded: 20.0C min and 30.0C max")

		monitor.extremes_changed(notification_trigger)
	end

	it "should not notify if the range falls within the limit" do
		notification.should_not_receive(:call)
		monitor.extremes_changed(pass)		
	end
end

describe 'Temperature Extreme Monitor' do
	let(:notification) { double("notification") }
	let(:monitor) do 
		monitor = TemperatureExtremeMonitor.new(:max => 32, :min => 20)
		monitor.when_extreme_exceeded(notification)
		monitor
	end

	let(:notification_trigger) do
		double("history",
			:max_temp => HistoryRecord.new('32.2C', DateTime.now), 
			:min_temp => HistoryRecord.new('22.2C', DateTime.now))
	end

	let(:min_trigger) do
		double("history",
			:max_temp => HistoryRecord.new('32.0C', DateTime.now), 
			:min_temp => HistoryRecord.new('19.9C', DateTime.now))
	end

	let(:pass) do
		double("history",
			:max_temp => HistoryRecord.new('32.0C', DateTime.now), 
			:min_temp => HistoryRecord.new('20.0C', DateTime.now))
	end

	it_behaves_like "A Monitor"

	it "should notify if the max is exceeded" do
		notification.should_receive(:call).with(
			"Temperature extremes of 32.0C - 20.0C exceeded: 32.2C")
		monitor.extremes_changed(notification_trigger)
	end

	it "should notify if the min is exceeded" do
		notification.should_receive(:call).with(
			"Temperature extremes of 32.0C - 20.0C exceeded: 19.9C")
		monitor.extremes_changed(min_trigger)
	end

	it "should not notify if the range falls within the limit" do
		notification.should_not_receive(:call)
		monitor.extremes_changed(pass)
	end
end