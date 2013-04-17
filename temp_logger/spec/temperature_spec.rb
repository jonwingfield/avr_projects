require 'weather_reading'

describe Temperature do
	it "should default to celcius" do
		Temperature.new(22.4).c.should == 22.4
	end

	it 'should convert to farenheit' do
		Temperature.new(25.3).f.should == 77.5
	end

	context "specifying farenheit as the unit" do
		it "should use farenheit instead of celicus" do
			Temperature.new("25.3F").f.should == 25.3
		end

		it "should convert to celcius" do
			Temperature.new('77.0f').c.should == 25.0
		end
	end

	context "specifying celcius as the unit" do
		it "should convert to farenheit" do
			Temperature.new("22.4C").c.should == 22.4
		end
	end

	context "comparing with other temperatures" do
		it ">" do
			Temperature.new("22.4C").should > Temperature.new("70.0F")
		end

		it "<" do
			Temperature.new("77.0f").should < Temperature.new("25.1C")
		end

		it "=" do
			Temperature.new("25.0c").should == Temperature.new("77.0F")
		end
	end
end