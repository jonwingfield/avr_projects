class Temperature
	attr_reader :c, :f

	def initialize(temp)
		if temp.instance_of? String
			temp.downcase!
			if temp.end_with? 'f'
				from_f temp.chop
			elsif temp.end_with? 'c'
				from_c temp.chop
			else
				raise "unrecognized temperature unit in #{temp}"
			end
		else
			from_c temp
		end
	end

	def self.from(thing)
		if thing.instance_of? Temperature
			thing
		else
			Temperature.new(thing)
		end
	end

	def >(temp)
		@c > temp.c
	end

	def <(temp)
		@c < temp.c
	end

	def ==(temp)
		@c == temp.c
	end

	def to_s
		"#{@c}C"
	end

private 
	def from_f temp
		@f = temp.to_f
		@c = (5.0/9.0 * (@f - 32)).round(1)
	end

	def from_c temp
		@c = temp.to_f
		@f = (9.0/5.0 * @c + 32).round(1)		
	end
end


class WeatherReading
	def self.from_sensor(data)
		values = data.split ','
		hashed = {}
		
		values.each do |val|
			kv = val.split ':'
			hashed[kv[0].strip] = kv[1].strip
		end

		temperature = hashed["Temperature"].to_f
                temperature = 45.0 if temperature > 45.0
		humidity = hashed["Humidity"].to_f
		humidity = 100.0 if humidity > 100.0

		return WeatherReading.new(temperature, humidity)		
	end

	attr_reader :temperature, :humidity, :dewpoint

	def initialize(temp, humidity)
		@temperature = Temperature.new(temp)
		@humidity = humidity
		@dewpoint = Temperature.new(calculate_dewpoint(temp, humidity))
	end

private
	
	def calculate_dewpoint(temp, humidity)
		b = 18.678
		c = 257.14
		gamma = Math.log(humidity / 100) + (b * temp) / (c + temp)
		dewpoint = (c * gamma) / (b - gamma)
		dewpoint.round(1)
	end
end

