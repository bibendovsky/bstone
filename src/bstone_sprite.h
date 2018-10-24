//
// A sprite.
//


#ifndef BSTONE_SPRITE_INCLUDED
#define BSTONE_SPRITE_INCLUDED


#include <cstdint>
#include <vector>


namespace bstone
{


class Sprite
{
public:
	// Sprite dimension.
	static constexpr auto side = 64;


	Sprite();

	Sprite(
		const Sprite& rhs);

	Sprite(
		Sprite&& rhs);

	Sprite& operator=(
		const Sprite& rhs);

	~Sprite();


	void initialize(
		const void* raw_data);

	void uninitialize();


	// Returns true if sprite is initialized.
	bool is_initialized() const;

	// Return an offset by x of the left edge.
	int get_left() const;

	// Return an offset by x of the right edge.
	int get_right() const;

	// Return an offset by y of the top edge.
	int get_top() const;

	// Return an offset by y of the bottm edge.
	int get_bottom() const;

	// Returns a width of the sprite.
	int get_width() const;

	// Returns a height of the sprite.
	int get_height() const;

	// Returns a pointer to the column data.
	//
	// Note: Negative value means transparency.
	const short* get_column(
		const int index) const;


private:
	using Image = std::vector<std::int16_t>;


	int left_;
	int right_;
	int top_;
	int bottom_;

	Image image_;
}; // Sprite


} // bstone


#endif // !BSTONE_SPRITE_INCLUDED
