import { fetchStorageSlots } from '$lib/server/controller';
import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const GET: RequestHandler = async ({ fetch }) => {
	try {
		const slots = await fetchStorageSlots(fetch);
		return json({ slots });
	} catch (error) {
		return json(
			{
				error: error instanceof Error ? error.message : 'Failed to fetch storage slots'
			},
			{ status: 503 }
		);
	}
};
